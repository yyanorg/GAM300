-- Resources/Scripts/GamePlay/GroundChaseState.lua
local AttackDirector = require("Gameplay.AttackDirector")

local ChaseState = {}

-- How far out a waiting enemy holds, as a multiple of its own melee range.
-- Far enough that it is not standing inside the player, close enough that it
-- is visibly part of the fight and can step in the moment a slot frees.
local WAIT_STANDOFF = 1.8

function ChaseState:Enter(ai)
    -- Optional: force first repath on enter
    --print("[GroundChaseState] ENTER")
    ai:ClearPath()
    ai:StopCC()
    ai._pathRepathT = 999

    ai._animator:SetBool("PlayerInDetectionRange", true)
    ai._animator:SetBool("PatrolEnabled", true)
    ai._footstepTimer = 0

    -- Play alert SFX once when first detecting player (entering chase)
    ai:_publishSFX("alert")
end

function ChaseState:Update(ai, dt)
    local dtSec = dt or 0
    if dtSec > 1.0 then dtSec = dtSec * 0.001 end
    if dtSec <= 0 then return end
    if dtSec > 0.05 then dtSec = 0.05 end

    local chaseSpd = ai.config.ChaseSpeed or 1.8
    local attackR, meleeR, diseng = ai:GetRanges()
    local d2 = ai:GetPlayerDistanceSq()

    -- Too far -> Patrol
    if d2 > (diseng * diseng) and not ai.aggressive then
        ai:StopCC()
        ai.fsm:Change("Patrol", ai.states.Patrol)
        return
    end

    local inRange = ai.IsMelee and (d2 < (meleeR * meleeR)) or
                    ((not ai.IsMelee) and (d2 <= (attackR * attackR)))

    if inRange then
        ai:StopCC()
        if ai.IsPassive then
            ai.fsm:Change("Idle", ai.states.Idle)
            return
        end
        -- Ask permission before committing. Without this every enemy that
        -- reaches range attacks at once, and past two attackers the player is
        -- in damage stun 72% of the time and their own inputs stop mattering.
        if AttackDirector.TryAcquire(ai) then
            ai.fsm:Change("Attack", ai.states.Attack)
            return
        end
        -- Denied. Stay in Chase and wait for an opening rather than standing
        -- in the player's face doing nothing, which is what a plain return
        -- would look like.
        ai._waitingForAttackSlot = true
    else
        ai._waitingForAttackSlot = false
    end

    -- A waiting enemy holds at stand-off and keeps facing the player. Letting
    -- it keep closing would pile the whole group into the same spot, so the
    -- fight would look identical to having no director at all even though
    -- only two of them are swinging.
    if ai._waitingForAttackSlot then
        local standoff = meleeR * WAIT_STANDOFF
        if d2 < (standoff * standoff) then
            ai:StopCC()
            ai:FacePlayer()
            return
        end
    end

    local tr = ai._playerTr
    if not tr then return end
    local pp = Engine.GetTransformPosition(tr)
    local px, pz = pp[1], pp[3]

    -- Repath conditions:
    -- 1) timed interval
    -- 2) player moved enough
    -- 3) stuck while following path
    local needRepath = ai:ShouldRepathToXZ(px, pz, dtSec)
    if (ai._pathStuckT or 0) >= (ai.PathStuckTime or 0.75) then
        needRepath = true
    end

    if needRepath then
        ai._pathRepathT = 0
        local pathFound = ai:RequestPathToXZ(px, pz)
        
        if not pathFound then
            --print("[Chase] NO PATH to player - stopping movement")
            ai:StopCC()
            return
        end
    end

    -- Follow the path
    local arrived = ai:FollowPath(dtSec, chaseSpd)

    -- If we "arrived" but still not in attack range (can happen if path ends early), do a soft fallback
    if arrived then
        ai:StopCC()
        ai._footstepTimer = 0
        -- optional: face player for aiming
        ai:FacePlayer()
    else
        ai._footstepTimer = (ai._footstepTimer or 0) + dtSec
        if ai._footstepTimer >= (ai.ChaseFootstepInterval or 0.35) then
            ai._footstepTimer = 0
            ai:_publishSFX("footstep")
        end
    end
end

return ChaseState

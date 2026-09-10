-- Resources/Scripts/GamePlay/FlyingChaseState.lua
local AttackDirector = require("Gameplay.AttackDirector")
local FlyingChase = {}

function FlyingChase:Enter(ai)
    ai._animator:SetBool("Flying", true)
    ai._animator:SetBool("PatrolEnabled", false)
    ai._animator:SetBool("PlayerInDetectionRange", true)
    ai._animator:SetBool("PlayerInAttackRange", false)
    ai._animator:SetBool("ReadyToAttack", false)
    ai:_publishSFX("alert")
end

function FlyingChase:Update(ai, dt)
    ai:MaintainHover(dt)

    -- Leash check: if too far from spawn, deaggro and fly home
    local maxDist = ai.MaxChaseDistance or 10.0
    if ai._spawnX and ai._spawnZ then
        local ex, _, ez = ai:GetPosition()
        if ex then
            local dx = ex - ai._spawnX
            local dz = ez - ai._spawnZ
            if (dx*dx + dz*dz) > (maxDist * maxDist) then
                ai.aggressive = false
                -- Point patrol target at spawn so the enemy flies home
                ai._patrolTarget = { x = ai._spawnX, z = ai._spawnZ }
                ai._isPatrolWait = false
                if ai.EnablePatrol then
                    ai.fsm:Change("Patrol", ai.states.Patrol)
                else
                    ai.fsm:Change("Idle", ai.states.Idle)
                end
                return
            end
        end
    end

    local detR = ai.DetectionRange or 4.0
    if not ai:IsPlayerInRange(detR) and not ai.aggressive then
        if ai.EnablePatrol then
            ai.fsm:Change("Patrol", ai.states.Patrol)
        else
            ai.fsm:Change("Idle", ai.states.Idle)
        end
        return
    end

    local attackR = ai.AttackRange or 3.0
    if ai:IsPlayerInRange(attackR) then
        -- Flyers draw on the same budget as everyone else. Measured with only
        -- the ground enemies gated, the statue room still reached four
        -- attackers against a cap of two, because the two flyers were never
        -- asking. A stun from above costs the player exactly as much as one
        -- from in front.
        if AttackDirector.TryAcquire(ai) then
            ai.fsm:Change("Attack", ai.states.Attack)
            return
        end
        -- No slot: keep circling. Falling through to the chase below is what
        -- a flyer waiting its turn should look like anyway.
    end

    -- chase
    local spd = ai.FlyingChaseSpeed or 1.2
    ai:MoveTowardPlayerXZ_Flying(dt, spd)
end

return FlyingChase
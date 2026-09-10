-- Resources/Scripts/GamePlay/GroundHurtState.lua
local AttackDirector = require("Gameplay.AttackDirector")
local HurtState = {}

function HurtState:Enter(ai)
    --print("[GroundHurtState] ENTER")
    ai:FacePlayer()

    --print("[GroundHurtState] FACED PLAYER")

    -- Lock out attacks briefly
    ai._hurtTimer = 0

    -- if ai.particles then
    --     ai.particles.isEmitting   = true
    --     ai.particles.emissionRate = 180
    -- end

    -- Hit feathers are spawned by EnemyAI:ApplyHit, which sees every landed hit.
    -- Spawning them here missed every hit that does not enter this state
    -- (juggle hits, feather-skill hits, and hits while hooked).
    --print("[GroundHurtState] SPAWNED FEATHERS")
end

function HurtState:Update(ai, dt)
    ai._hurtTimer = (ai._hurtTimer or 0) + dt

    -- Wait for hurt "stun" duration, then return to appropriate state
    if ai._hurtTimer >= ai.config.HurtDuration then
        ai._hurtTimer = 0

        if ai.health <= 0 then
            self.dead = true
            return
        end

        -- If player is still near, resume Attack. Otherwise go Idle.
        if ai:IsPlayerInRange(ai.config.DetectionRange) then
            if ai.IsPassive then
                ai.fsm:Change("Idle", ai.states.Idle)
            elseif AttackDirector.TryAcquire(ai) then
                -- Recovering from a hit is not a licence to skip the queue.
                -- This path went straight back into Attack, so an enemy that
                -- was being hit could re-enter above the cap and the crowd
                -- could still take the controls away.
                ai.fsm:Change("Attack", ai.states.Attack)
            else
                ai.fsm:Change("Chase", ai.states.Chase)
            end
        else
            ai.fsm:Change("Idle", ai.states.Idle)
        end
    end
end

function HurtState:Exit(ai)
    --print("[GroundHurtState] Animator:SetBool(Hurt, false)")
    ai._animator:SetBool("Hurt1", false)
    ai._animator:SetBool("Hurt2", false)
    ai._animator:SetBool("Hurt3", false)
    -- if ai.particles then
    --     ai.particles.isEmitting   = false
    --     ai.particles.emissionRate = 0
    -- end
end

return HurtState

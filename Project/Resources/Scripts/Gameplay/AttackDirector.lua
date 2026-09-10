-- Decides how many enemies may attack the player at once.
--
-- Nothing coordinated this before. Every enemy that reached melee range
-- entered Attack independently, so a group attacked as fast as its members
-- happened to arrive. Measured on the route: the player is held in damage
-- stun 35% of the time against one enemy, 72% against two and 75% against
-- three. Past two enemies the fight stops being a fight and becomes a
-- stunlock, and the player's own inputs stop mattering, which is the thing
-- that makes it feel bad rather than hard.
--
-- The stun cooldown added earlier caps how often a single hit can re-apply
-- the lock. It treats the symptom. This is the cause: too many attackers.
--
-- A permission slot is taken before entering Attack and given back on the way
-- out. An enemy that cannot get one keeps chasing but holds at stand-off
-- distance instead of crowding in, which is what a group of attackers reads
-- as when it works: some pressing, the rest circling and waiting for an
-- opening.
--
-- No timers and no clock. Lua here has no engine time source, os.clock is
-- process CPU time, and a lease that expires on the wrong clock either frees
-- a slot mid-swing or holds one forever. Instead every route out of Attack
-- releases explicitly, and dead holders are purged whenever a slot is asked
-- for, so a slot cannot outlive the enemy holding it.

local AttackDirector = {}

-- How many enemies may be in Attack at once. Two is the measured knee: one
-- enemy leaves the player in control, three is a stunlock.
--
-- GAM300_MAX_ATTACKERS overrides it, which is how the before and after were
-- measured against each other in one build: set high enough that nothing is
-- ever refused and the behaviour is identical to having no director at all,
-- which is the only honest baseline.
AttackDirector.MaxConcurrent = 2
pcall(function()
    local n = tonumber(os.getenv("GAM300_MAX_ATTACKERS"))
    if n and n > 0 then AttackDirector.MaxConcurrent = n end
end)

-- Held slots, keyed by the ai table itself so no id scheme is needed and two
-- enemies can never collide on a key.
local holders = {}
local heldCount = 0

-- Enemies turned away since the last grant. Used only to decide whether a
-- slot needs to rotate: if nobody was waiting, the enemy holding it can carry
-- straight on.
local refused = {}

-- Enemies that must yield their next request. An attacker that finishes a
-- swing while others are waiting goes in here, so the slot passes on instead
-- of being immediately retaken by the same enemy.
local backoff = {}

local function isDead(ai)
    local ok, dead = pcall(function()
        return ai.dead or ai.isDead or false
    end)
    return (not ok) or dead
end

-- Drop slots whose holder has died. Death does not always run an Exit, and a
-- slot held by a corpse would shut the rest of the group out of the fight for
-- the remainder of the encounter.
local function purge()
    for ai in pairs(holders) do
        if isDead(ai) then
            holders[ai] = nil
            heldCount = heldCount - 1
        end
    end
end

-- Slots are dropped wholesale on a respawn. The holders table lives in the
-- Lua VM, which outlives a scene, so an enemy still holding a slot when the
-- level reloads would keep it forever and shut the next encounter out of
-- attacking entirely. Subscribed lazily because this module is required while
-- scripts are still loading, before the event bus exists.
local subscribed = false
local function ensureSubscribed()
    if subscribed then return end
    if not (_G.event_bus and _G.event_bus.subscribe) then return end
    subscribed = true
    pcall(function()
        _G.event_bus.subscribe("respawnPlayer", function()
            AttackDirector.Reset()
        end)
    end)
end

--- True if this enemy may attack, taking a slot if one is free.
-- Re-asking while already holding is free, so a state can call this every
-- frame without leaking slots.
function AttackDirector.TryAcquire(ai)
    if ai == nil then return true end
    ensureSubscribed()
    if holders[ai] then return true end

    purge()

    -- Yield once, so the slot this enemy just gave up goes to one of the
    -- enemies that were waiting for it rather than straight back.
    if backoff[ai] then
        backoff[ai] = nil
        refused[ai] = true
        return false
    end

    if heldCount >= AttackDirector.MaxConcurrent then
        refused[ai] = true
        return false
    end

    refused[ai] = nil
    holders[ai] = true
    heldCount = heldCount + 1
    return true
end

--- Give the slot back. Safe to call when not holding one.
function AttackDirector.Release(ai)
    if ai ~= nil and holders[ai] then
        holders[ai] = nil
        heldCount = heldCount - 1
        -- Only rotate when somebody is actually waiting. With one enemy left
        -- alive there is nobody to pass to, and making it stand and yield
        -- between every swing would turn the last enemy of a fight into a
        -- punching bag.
        if next(refused) ~= nil then
            backoff[ai] = true
        end
    end
end

--- How many slots are currently held. For telemetry and tests.
function AttackDirector.Count()
    purge()
    return heldCount
end

--- Drop every slot. For a scene change or a player death, where the whole
--- encounter is being torn down and Exit may not run for anyone.
function AttackDirector.Reset()
    holders = {}
    heldCount = 0
    refused = {}
    backoff = {}
end

-- Published so telemetry can read how many enemies are attacking without
-- reaching into this module.
_G.AttackDirector = AttackDirector

return AttackDirector

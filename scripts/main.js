import { world, system } from "@minecraft/server";

// Keep track so the notification only shows once when you enter a world
let hasNotified = false;

world.afterEvents.playerSpawn.subscribe((event) => {
    if (hasNotified) return;
    
    const player = event.player;
    
    // Shows "[Entity Culling: Activated]" right above your hotbar
    player.runCommand(`title @s actionbar §a§l[Entity Culling: Activated]`);
    
    // Plays the official high-pitched success "ding" sound
    player.runCommand(`playsound random.toast @s ~ ~ ~ 1.0 1.0`);
    
    hasNotified = true; 
});

// High-performance optimization loop running every 20 ticks (1 second)
system.runInterval(() => {
    const players = world.getAllPlayers();
    if (players.length === 0) return;

    const localPlayer = players[0];
    const playerPos = localPlayer.location;
    const entities = localPlayer.dimension.getEntities();

    for (const entity of entities) {
        // We never cull players!
        if (entity.typeId === "minecraft:player") continue;

        const entityPos = entity.location;
        const dx = entityPos.x - playerPos.x;
        const dy = entityPos.y - playerPos.y;
        const dz = entityPos.z - playerPos.z;
        const distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        // 40 Blocks Culling boundary (40 * 40 = 1600 distance squared)
        if (distanceSq > 1600) {
            // Forces the game engine to unload/despawn the entity to instantly free up memory
            entity.triggerEvent("minecraft:despawn"); 
        }
    }
}, 20);

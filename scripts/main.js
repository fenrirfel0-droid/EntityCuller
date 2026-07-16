import { world, system } from "@minecraft/server";

// Shows "[Entity Culling: ON]" on your screen when you spawn
world.afterEvents.playerSpawn.subscribe((event) => {
    const player = event.player;
    player.onScreenDisplay.setActionBar("§a§l[Entity Culling: ON]");
});

// Optimization loop: despawns distant mobs to save your FPS
system.runInterval(() => {
    const players = world.getAllPlayers();
    if (players.length === 0) return;

    const localPlayer = players[0];
    const playerPos = localPlayer.location;
    const entities = localPlayer.dimension.getEntities();

    for (const entity of entities) {
        if (entity.typeId === "minecraft:player") continue;

        const entityPos = entity.location;
        const dx = entityPos.x - playerPos.x;
        const dy = entityPos.y - playerPos.y;
        const dz = entityPos.z - playerPos.z;
        const distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        // Past 40 blocks, optimize them out to boost FPS
        if (distanceSq > 1600) {
            entity.triggerEvent("minecraft:despawn"); 
        }
    }
}, 20); // Runs once every second

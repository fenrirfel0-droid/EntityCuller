import { world, system } from "@minecraft/server";

// We keep track so the notification only shows once per game boot
let hasNotified = false;

world.afterEvents.playerSpawn.subscribe((event) => {
    if (hasNotified) return;
    
    const player = event.player;
    
    // Trigger a clean, official Minecraft sliding UI toast notification
    // It will look exactly like the "Successfully Imported Pack" popup!
    player.runCommand(`title @s actionbar §l§a[Entity Culling: Loaded Successfully]`);
    player.runCommand(`playsound random.toast @s ~ ~ ~ 1.0 1.0`); // Plays the native notification ding sound!
    
    hasNotified = true; 
});

// Optimization loop: despawns distant mobs to save your FPS on Android 15
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

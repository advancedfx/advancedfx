// Purpose:
// Show practical example how modify game events in hook
// Can be combined with other examples to send data to server
// Here it's bare minimal with output to game's console
//
// Say we want to have steamIds and names of players in the player_death event
// but the original event doesn't provide it, so we enrich it here
// See event reference:
// https://cs2.poggu.me/dumped-data/game-events#player_death
//
// Usage:
// 1) Load this script in game with mirv_script_load
// e.g. mirv_script_load "C:\advancedfx\misc\mirv-script\dist\examples\2-enrich-game-events\index.js"
//
// 2) Load demo, fast forward to player_death event,
// expect JSON string with event data in console.

/* eslint-disable @typescript-eslint/naming-convention */
{
	// Patch the prototype, so it's handled correctly in stringify function
	BigInt.prototype.toJSON = function () {
		return this.toString();
	};
	// Define hook. We set it later in the end
	const onGameEvent: mirv.OnGameEvent = (e) => {
		if (e.name === 'player_death') {
			let attacker_steamid: bigint | null = null;
			let attacker_name: string | null = null;
			let assister_steamid: bigint | null = null;
			let assister_name: string | null = null;
			let user_steamid: bigint | null = null;
			let user_name: string | null = null;

			type PartialPlayerDeath = {
				attacker: number;
				assister: number;
				userid: number;
			};
			const rawData = JSON.parse(e.data) as PartialPlayerDeath;

			const UINT16_MAX = 0xffff; // 65535
			// 65535 is invalid id, meaning there is no such entity
			// Usually it's only the case for assister in player_death event, meaning there was no assister.
			// But we can check them all for good measure.
			if (rawData.attacker !== UINT16_MAX) {
				// id + 1 = entity entry index
				const attackerController = mirv.getEntityFromIndex(rawData.attacker + 1);
				// Check for player controller here is not mandatory since we already know that it is
				if (attackerController && attackerController.isPlayerController()) {
					// Note:
					// SteamId is returned as bigint and has to be converted to string because javascript
					// We don't convert it here here inline, because we patched the prototype above,
					// so its automatic when we stringify entire thing later
					attacker_steamid = attackerController.getSteamId();
					attacker_name = attackerController.getSanitizedPlayerName();
				}
			}

			if (rawData.assister !== UINT16_MAX) {
				const assisterController = mirv.getEntityFromIndex(rawData.assister + 1);
				if (assisterController && assisterController.isPlayerController()) {
					assister_steamid = assisterController.getSteamId();
					assister_name = assisterController.getSanitizedPlayerName();
				}
			}

			if (rawData.userid !== UINT16_MAX) {
				const userController = mirv.getEntityFromIndex(rawData.userid + 1);
				if (userController && userController.isPlayerController()) {
					user_steamid = userController.getSteamId();
					user_name = userController.getSanitizedPlayerName();
				}
			}

			// Add new fields
			const newData = {
				...rawData,
				attacker_steamid,
				attacker_name,
				assister_steamid,
				assister_name,
				user_steamid,
				user_name
			};

			// Convert it back to JSON
			// or can be js object if you stringify/send whole object later
			e.data = JSON.stringify(newData);

			// Event can be send over to server if needed
			// Here we just logging it to game's console
			console.log(`name: "${e.name}"`, `id: "${e.id}"`, `data: "${e.data}"`);
		}
	};

	// Set the hook. (Can be also set inline like in other examples.)
	//
	// Make sure this hook doesn't get overwritten elsewhere
	// since currently HLAE doesn't handle such conflicts
	mirv.onGameEvent = onGameEvent;
}

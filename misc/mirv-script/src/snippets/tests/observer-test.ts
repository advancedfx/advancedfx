{
	const localControllerEntity = mirv.getEntityFromSplitScreenPlayer(0);
	if (null !== localControllerEntity) {
		const localPawnEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localControllerEntity.getPlayerPawnHandle())
		);
		if (null !== localPawnEntity) {
			mirv.message(`Observer mode: ${localPawnEntity.getObserverMode()}\n`);
			const observerTargetEntity = mirv.getEntityFromIndex(
				mirv.getHandleEntryIndex(localPawnEntity.getObserverTargetHandle())
			);
			if (null !== observerTargetEntity) {
				const obserTargetControllerEntity = mirv.getEntityFromIndex(
					mirv.getHandleEntryIndex(observerTargetEntity.getPlayerControllerHandle())
				);
				if (null !== obserTargetControllerEntity) {
					mirv.message(
						`Observed player name: ${obserTargetControllerEntity.getSanitizedPlayerName()}\n`
					);
				}
				const activeWeaponEntity = mirv.getEntityFromIndex(
					mirv.getHandleEntryIndex(observerTargetEntity.getActiveWeaponHandle())
				);
				if (null !== activeWeaponEntity) {
					mirv.message(`Observed player weapon: ${activeWeaponEntity.getDebugName()}\n`);
				}
			}
		}
	}
}

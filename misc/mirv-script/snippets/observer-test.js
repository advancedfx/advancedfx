//var highest = mirv.getHighestEntityIndex();
//for(var i=0; i < highest + 1; i++) {
{
	var localControllerEntity = mirv.getEntityFromSplitScreenPlayer(0);
	if (null !== localControllerEntity) {
		var localPawnEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localControllerEntity.getPlayerPawnHandle())
		);
		if (null !== localPawnEntity) {
			mirv.message('Observer mode: ' + localPawnEntity.getObserverMode() + '\n');
			var observerTargetEntity = mirv.getEntityFromIndex(
				mirv.getHandleEntryIndex(localPawnEntity.getObserverTargetHandle())
			);
			if (null !== observerTargetEntity) {
				var obserTargetControllerEntity = mirv.getEntityFromIndex(
					mirv.getHandleEntryIndex(observerTargetEntity.getPlayerControllerHandle())
				);
				if (null !== obserTargetControllerEntity) {
					mirv.message(
						'Observed player name: ' +
							obserTargetControllerEntity.getSanitizedPlayerName() +
							'\n'
					);
				}
				var activeWeaponEntity = mirv.getEntityFromIndex(
					mirv.getHandleEntryIndex(observerTargetEntity.getActiveWeaponHandle())
				);
				if (null !== activeWeaponEntity) {
					mirv.message(
						'Observed player weapon: ' + activeWeaponEntity.getDebugName() + '\n'
					);
				}
			}
		}
	}
}

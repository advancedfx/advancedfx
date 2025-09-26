{
	const localControllerEntity = mirv.getEntityFromSplitScreenPlayer(0);

	if (null !== localControllerEntity) {
		const localPawnEntity = mirv.getEntityFromIndex(
			mirv.getHandleEntryIndex(localControllerEntity.getPlayerPawnHandle())
		);

		if (null !== localPawnEntity) {
			const observerTargetEntity = mirv.getEntityFromIndex(
				mirv.getHandleEntryIndex(localPawnEntity.getObserverTargetHandle())
			);

			if (null !== observerTargetEntity) {
				const attachment = observerTargetEntity.getAttachment('weapon_hand_r');

				if (null !== attachment) {
					const { position, angles } = attachment;
					const anglesEuler = angles.toQREulerAngles().toQEulerAngles();

					console.log('position (x y z)', position.x, position.y, position.z);
					console.log(
						'angles (pitch yaw roll)',
						anglesEuler.pitch,
						anglesEuler.yaw,
						anglesEuler.roll
					);
				}
			}
		}
	}
}

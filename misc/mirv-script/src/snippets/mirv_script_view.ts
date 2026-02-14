{
    String.prototype.dedent = function () {
        return this.split('\n')
            .map((l) => l.trim())
            .join('\n');
    };
    let index = -1;
    let last_spec_mode: number | null = null;
    let last_spec_handle: number | null = null;
    const re_int = /^(\-)?[0-9]+$/;

    // @ts-ignore
    if (mirv.mirv_script_view !== undefined) mirv.mirv_script_view.unregister();
    // @ts-ignore
    mirv.mirv_script_view = new AdvancedfxConCommand((args) => {
        const argC = args.argC();
        const arg0 = args.argV(0);
        if (2 <= argC) {
            const arg1 = args.argV(1).toLowerCase();
            if ('list' === arg1) {
                for (let i = 0; i < 64; i++) {
                    const entity = mirv.getEntityFromIndex(i + 1);
                    if (null !== entity && entity.isPlayerController()) {
                        const handle = entity.getPlayerPawnHandle();
                        if (mirv.isHandleValid(handle)) {
                            const idx = mirv.getHandleEntryIndex(handle);
                            mirv.message(`${idx} : ${entity.getSanitizedPlayerName()}\n`);
                        }
                    }
                }
                return;
            } else if (arg1.match(re_int)) {
                const index = parseInt(arg1);
                if (index === -1) {
                    mirv.onCViewRenderSetupView = undefined;
                    mirv.onClientFrameStageNotify = undefined;
                    return;
                } else {
                    last_spec_mode = null;
                    last_spec_handle = null;
                    mirv.onClientFrameStageNotify = (e) => {
                        if (e.curStage == 5 && !e.isBefore) {
                            const entity = mirv.getEntityFromIndex(index);
                            if (null !== entity) {
                                const spec_mode = entity.getObserverMode();
                                const spec_handle = entity.getObserverTargetHandle();
                                if (null === last_spec_mode || last_spec_mode !== spec_mode) mirv.exec('spec_mode ' + spec_mode);
                                if (null === last_spec_handle || last_spec_handle !== spec_handle) {
                                    const handle_index = mirv.getHandleEntryIndex(spec_handle);
                                    const entity = mirv.getEntityFromIndex(handle_index);
                                    if (null !== entity) {
                                        if (entity.isPlayerPawn()) {
                                            const player_handle = entity.getPlayerControllerHandle();
                                            const player_index = mirv.getHandleEntryIndex(player_handle)
                                            mirv.exec('spec_player ' + player_index);
                                        }
                                    }
                                }
                                last_spec_mode = spec_mode;
                                last_spec_handle = spec_handle;
                            }
                        }
                    }
                    mirv.onCViewRenderSetupView = (e) => {
                        if (last_spec_mode == 2) return; // don't override in in-eye spec
                        const entity = mirv.getEntityFromIndex(index);
                        if (null !== entity) {
                            const eyeOrigin = entity.getRenderEyeOrigin();
                            const eyeAngles = entity.getRenderEyeAngles();
                            return {
                                x: eyeOrigin[0],
                                y: eyeOrigin[1],
                                z: eyeOrigin[2],
                                rX: eyeAngles[0],
                                rY: eyeAngles[1],
                                rZ: eyeAngles[2]
                            };
                        }
                    };
                    return;
                }
            }
        }
        mirv.message(`Usage:
			${arg0} <i> - Show view of entity with index <i>.
			${arg0} -1 - Deactivate.
			${arg0} list - Show list of player pawn entites.
			Current value: ${index}
			`.dedent());
    });
    // @ts-ignore
    mirv.mirv_script_view.register('mirv_script_view', 'Show entites\' view by index');
}

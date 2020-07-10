#include "stdafx.h"

#include "MirvCamPath.h"

#include "StringTools.h"

#include <sstream>
#include <iomanip>

void MirvCampath_PrintTimeFormated(double time, advancedfx::Con_Printf_t conMessage)
{
	int seconds = (int)time % 60;

	time /= 60;
	int minutes = (int)time % 60;

	time /= 60;
	int hours = (int)time;

	std::ostringstream oss;

	oss << std::setfill('0') << std::setw(2);

	if (hours)
	{
		oss << hours << "h";
	}

	oss << minutes << "m" << seconds << "s";

	conMessage("%s", oss.str().c_str());
}

void MirvCampath_ConCommand(advancedfx::ICommandArgs* args, advancedfx::Con_Printf_t conMessage, advancedfx::Con_Printf_t conWarning, CamPath* camPath, IMirvCampath_Time* mirvTime, IMirvCampath_Camera* mirvCamera, IMirvCampath_Drawer* mirvDrawer)
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const* subcmd = args->ArgV(1);

		if (!_stricmp("add", subcmd) && 2 == argc)
		{
			SMirvCameraValue camera = mirvCamera->GetCamera();

			camPath->Add(
				mirvTime->GetTime() - camPath->GetOffset(),
				CamPathValue(camera.X, camera.Y, camera.Z, camera.Pitch, camera.Yaw, camera.Roll, camera.Fov)
			);

			return;
		}
		else if (!_stricmp("enabled", subcmd) && 3 == argc
			|| !_stricmp("enable", subcmd) && 3 == argc)
		{
			bool enable = 0 != atoi(args->ArgV(2));
			camPath->Enabled_set(enable);

			if (enable && !camPath->CanEval())
				conWarning(
					"Warning: Campath enabled but can not be evaluated yet.\n"
					"Did you add enough points?\n"
				);

			return;
		}
		else if (!_stricmp("draw", subcmd) && mirvDrawer)
		{
			if (3 <= argc)
			{
				char const* cmd2 = args->ArgV(2);

				if (!_stricmp("enabled", cmd2))
				{
					if (4 <= argc)
					{
						char const* cmd3 = args->ArgV(3);

						bool enabled = 0 != atoi(cmd3);

						mirvDrawer->SetEnabled(enabled);
						return;
					}

					conMessage(
						"%s draw enabled 0|1 - enable (1) / disable (0) drawing.\n"
						"Current value: %s\n",
						args->ArgV(0),
						mirvDrawer->GetEnabled() ? "1 (enabled)" : "0 (disabled)"
					);
					return;
				}
				else if (0 == _stricmp("keyAxis", cmd2))
				{
					if (4 <= argc)
					{
						char const* cmd3 = args->ArgV(3);

						bool enabled = 0 != atoi(cmd3);

						mirvDrawer->SetDrawKeyframeAxis(enabled);
						return;
					}

					conMessage(
						"%s draw keyAxis 0|1 - enable (1) / disable (0) drawing.\n"
						"Current value: %s\n",
						args->ArgV(0),
						mirvDrawer->GetDrawKeyframeAxis() ? "1 (enabled)" : "0 (disabled)"
					);
					return;
				}
				else if (0 == _stricmp("keyCam", cmd2))
				{
					if (4 <= argc)
					{
						char const* cmd3 = args->ArgV(3);

						bool enabled = 0 != atoi(cmd3);

						mirvDrawer->SetDrawKeyframeCam(enabled);
						return;
					}

					conMessage(
						"%s draw keyCam 0|1 - enable (1) / disable (0) drawing.\n"
						"Current value: %s\n",
						args->ArgV(0),
						mirvDrawer->GetDrawKeyframeCam() ? "1 (enabled)" : "0 (disabled)"
					);
					return;
				}
				else if (0 == _stricmp("keyIndex", cmd2))
				{
					if (4 <= argc)
					{
						char const* cmd3 = args->ArgV(3);

						mirvDrawer->SetDrawKeyframeIndex(atof(cmd3));
						return;
					}

					conMessage(
						"%s draw keyIndex <fHeight> - The text height.\n"
						"Current value: %f\n",
						args->ArgV(0),
						mirvDrawer->GetDrawKeyframeIndex()
					);
					return;
				}
			}

			conMessage("%s draw enabled [...] - enable / disable drawing.\n", args->ArgV(0));
			conMessage("%s draw keyAxis [...] - If to draw axis at keyframes.\n", args->ArgV(0));
			conMessage("%s draw keyCam [...] - If to draw camera at keyframes.\n", args->ArgV(0));
			conMessage("%s draw keyIndex [...] - Size of keyframe index to draw.\n", args->ArgV(0));
			return;
		}
		else if (!_stricmp("clear", subcmd) && 2 == argc)
		{
			camPath->Clear();
			return;
		}
		else if (!_stricmp("print", subcmd) && 2 == argc)
		{
			conMessage("passed? selected? id: tick[offset](approximate!), demoTime[offset](approximate!), gameTime[offset] -> (x,y,z) fov (pitch,yaw,roll)\n");

			double curtime = mirvTime->GetTime();

			double offset = camPath->GetOffset();

			int i = 0;
			for (CamPathIterator it = camPath->GetBegin(); it != camPath->GetEnd(); ++it)
			{
				double vieworigin[3];
				double viewangles[3];
				double fov;

				double time = it.GetTime();
				CamPathValue val = it.GetValue();
				bool selected = val.Selected;
				QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

				vieworigin[0] = val.X;
				vieworigin[1] = val.Y;
				vieworigin[2] = val.Z;
				viewangles[0] = ang.Pitch;
				viewangles[1] = ang.Yaw;
				viewangles[2] = ang.Roll;
				fov = val.Fov;

				conMessage(
					"%s %s %i: ",
					time <= curtime + offset ? "Y" : "n",
					selected ? "Y" : "n",
					i
				);

				if (offset)
				{
					int myTick, myTickOffset;
					if (mirvTime->GetDemoTickFromTime(curtime, time, myTick) && mirvTime->GetDemoTickFromTime(curtime + offset, time, myTickOffset))
					{
						int delta = myTickOffset - myTick;
						conMessage("%i%s%i", myTick, delta < 0 ? "" : "+", delta);
					}
					else
						conMessage("n/a");
				}
				else
				{
					int myTick;
					if (mirvTime->GetDemoTickFromTime(curtime, time, myTick))
						conMessage("%i", myTick);
					else
						conMessage("n/a");
				}

				conMessage(", ");

				double myDemoTime;
				if (mirvTime->GetDemoTimeFromTime(curtime, time, myDemoTime))
				{
					MirvCampath_PrintTimeFormated(myDemoTime, conMessage);
					if (offset > 0)
					{
						conMessage("+");
					}
					if (offset)
					{
						MirvCampath_PrintTimeFormated(offset, conMessage);
					}
				}
				else
					conMessage("n/a");

				if (offset > 0)
				{
					conMessage(", %f+%f -> (%f,%f,%f) %f (%f,%f,%f)\n",
						time, offset,
						vieworigin[0], vieworigin[1], vieworigin[2],
						fov,
						viewangles[0], viewangles[1], viewangles[2]
					);
				}
				else if (offset < 0)
				{
					conMessage(", %f%f -> (%f,%f,%f) %f (%f,%f,%f)\n",
						time, offset,
						vieworigin[0], vieworigin[1], vieworigin[2],
						fov,
						viewangles[0], viewangles[1], viewangles[2]
					);
				}
				else
				{
					conMessage(", %f -> (%f,%f,%f) %f (%f,%f,%f)\n",
						time,
						vieworigin[0], vieworigin[1], vieworigin[2],
						fov,
						viewangles[0], viewangles[1], viewangles[2]
					);
				}

				i++;
			}

			conMessage("----\n");

			if (offset)
			{
				conMessage("Current offset: %s%f, ", offset < 0 ? "" : "+", offset);
			}

			conMessage("Current tick: ");
			int curTick;
			bool hasCurTick;
			if (hasCurTick = mirvTime->GetCurrentDemoTick(curTick))
				conMessage("%i", curTick);
			else
				conMessage("n/a");
			conMessage(", Current demoTime: ");
			double curDemoTime;
			if (hasCurTick && mirvTime->GetCurrentDemoTime(curDemoTime))
				MirvCampath_PrintTimeFormated(curDemoTime, conMessage);
			else
				conMessage("n/a");
			conMessage(", Current gameTime: %f\n", curtime);

			SMirvCameraValue camera = mirvCamera->GetCamera();

			conMessage("Current (x,y,z) fov (pitch,yaw,roll): (%f,%f,%f), %f, (%f,%f,%f)\n",
				camera.X,
				camera.Y,
				camera.Z,
				camera.Fov,
				camera.Pitch,
				camera.Roll,
				camera.Yaw
			);

			return;
		}
		else if (!_stricmp("remove", subcmd) && 3 == argc)
		{
			int idx = atoi(args->ArgV(2));
			int i = 0;
			for (CamPathIterator it = camPath->GetBegin(); it != camPath->GetEnd(); ++it)
			{
				if (i == idx)
				{
					double time = it.GetTime();
					camPath->Remove(time);
					break;
				}
				i++;
			}

			return;
		}
		else if (!_stricmp("load", subcmd) && 3 == argc)
		{
			std::wstring wideString;
			bool bOk = UTF8StringToWideString(args->ArgV(2), wideString)
				&& camPath->Load(wideString.c_str())
				;

			if(bOk) conMessage("Loading campath: %s.\n","OK");
			else conWarning("Loading campath: %s.\n", "ERROR");

			return;
		}
		else if (!_stricmp("save", subcmd) && 3 == argc)
		{
			std::wstring wideString;
			bool bOk = UTF8StringToWideString(args->ArgV(2), wideString)
				&& camPath->Save(wideString.c_str())
				;

			if (bOk) conMessage("Saving campath: %s.\n", "OK");
			else conWarning("Saving campath: %s.\n", "ERROR");

			return;
		}
		else if (!_stricmp("edit", subcmd))
		{
			if (3 <= argc)
			{
				const char* arg2 = args->ArgV(2);

				if (!_stricmp("start", arg2))
				{
					if (3 == argc)
					{
						camPath->SetStart(
							mirvTime->GetTime() - camPath->GetOffset()
						);

						return;
					}
					else
						if (3 < argc)
						{
							const char* arg3 = args->ArgV(3);

							if (!_stricmp("abs", arg3) && 5 <= argc)
							{
								char const* arg4 = args->ArgV(4);

								camPath->SetStart(
									atof(arg4) - camPath->GetOffset()
								);

								return;
							}
							else
								if (StringBeginsWith(arg3, "delta") && 4 == argc)
								{
									if (StringBeginsWith(arg3, "delta+"))
									{
										arg3 += strlen("delta+");

										camPath->SetStart(
											atof(arg3)
											, true
										);

										return;
									}
									else
										if (StringBeginsWith(arg3, "delta-"))
										{
											arg3 += strlen("delta-");

											camPath->SetStart(
												-atof(arg3)
												, true
											);

											return;
										}
								}
						}

				}
				else
					if (!_stricmp("duration", arg2) && 4 <= argc)
					{
						double duration = atof(args->ArgV(3));

						camPath->SetDuration(
							duration
						);

						return;
					}
					else
						if (!_stricmp(arg2, "position") && 4 <= argc)
						{
							char const* arg3 = args->ArgV(3);

							if (!_stricmp("current", arg3))
							{
								SMirvCameraValue camera = mirvCamera->GetCamera();

								camPath->SetPosition(
									camera.X,
									camera.Y,
									camera.Z
								);

								return;
							}
							else
								if (6 <= argc)
								{
									char const* arg4 = args->ArgV(4);
									char const* arg5 = args->ArgV(5);
									camPath->SetPosition(
										atof(arg3),
										atof(arg4),
										atof(arg5)
									);

									return;
								}
						}
						else
							if (!_stricmp(arg2, "angles") && 4 <= argc)
							{
								char const* arg3 = args->ArgV(3);

								if (!_stricmp("current", arg3))
								{
									SMirvCameraValue camera = mirvCamera->GetCamera();

									camPath->SetAngles(
										camera.Pitch,
										camera.Yaw,
										camera.Roll
									);

									return;
								}
								else
									if (6 <= argc)
									{
										char const* arg4 = args->ArgV(4);
										char const* arg5 = args->ArgV(5);
										camPath->SetAngles(
											atof(arg3),
											atof(arg4),
											atof(arg5)
										);

										return;
									}
							}
							else
								if (!_stricmp(arg2, "fov") && 4 <= argc)
								{
									char const* arg3 = args->ArgV(3);

									if (!_stricmp("current", arg3))
									{
										SMirvCameraValue camera = mirvCamera->GetCamera();

										camPath->SetFov(
											camera.Fov
										);

										return;
									}
									else
									{
										camPath->SetFov(
											atof(arg3)
										);

										return;
									}
								}
								else
									if (!_stricmp(arg2, "rotate") && 6 <= argc)
									{
										char const* arg3 = args->ArgV(3);
										char const* arg4 = args->ArgV(4);
										char const* arg5 = args->ArgV(5);

										camPath->Rotate(
											atof(arg3),
											atof(arg4),
											atof(arg5)
										);
										return;
									}
									else if (!_stricmp(arg2, "anchor"))
									{
										int argOfs = 3;

										if (argOfs < argc)
										{
											bool bOk = true;

											double anchorX;
											double anchorY;
											double anchorZ;
											double anchorYPitch;
											double anchorZYaw;
											double anchorXRoll;
											double destX;
											double destY;
											double destZ;
											double destYPitch;
											double destZYaw;
											double destXRoll;

											char const* curArg = args->ArgV(argOfs);

											if (StringBeginsWith(curArg, "#"))
											{
												curArg += 1;
												argOfs += 1;

												int anchorId = atoi(curArg);

												if (0 <= anchorId && anchorId < (int)camPath->GetSize())
												{
													int itId = 0;
													for (CamPathIterator it = camPath->GetBegin(); it != camPath->GetEnd(); ++it)
													{
														if (itId == anchorId)
														{
															CamPathValue val = it.GetValue();

															anchorX = val.X;
															anchorY = val.Y;
															anchorZ = val.Z;

															Afx::Math::QEulerAngles angs = val.R.ToQREulerAngles().ToQEulerAngles();

															anchorYPitch = angs.Pitch;
															anchorZYaw = angs.Yaw;
															anchorXRoll = angs.Roll;

															break;
														}

														++itId;
													}
												}
												else
													bOk = false;
											}
											else if (argOfs + 5 < argc)
											{
												anchorX = atof(curArg);
												anchorY = atof(args->ArgV(argOfs + 1));
												anchorZ = atof(args->ArgV(argOfs + 2));
												anchorYPitch = atof(args->ArgV(argOfs + 3));
												anchorZYaw = atof(args->ArgV(argOfs + 4));
												anchorXRoll = atof(args->ArgV(argOfs + 5));

												argOfs += 6;
											}
											else
												bOk = false;

											if (bOk && argOfs < argc)
											{
												char const* curArg = args->ArgV(argOfs);

												if (!_stricmp("current", curArg))
												{
													SMirvCameraValue camera = mirvCamera->GetCamera();

													destX = camera.X;
													destY = camera.Y;
													destZ = camera.Z;
													destYPitch = camera.Pitch;
													destZYaw = camera.Yaw;
													destXRoll = camera.Roll;

													argOfs += 1;
												}
												else if (argOfs + 5 < argc)
												{
													destX = atof(curArg);
													destY = atof(args->ArgV(argOfs + 1));
													destZ = atof(args->ArgV(argOfs + 2));
													destYPitch = atof(args->ArgV(argOfs + 3));
													destZYaw = atof(args->ArgV(argOfs + 4));
													destXRoll = atof(args->ArgV(argOfs + 5));

													argOfs += 6;
												}
												else
													bOk = false;

												if (bOk && argOfs == argc)
												{
													camPath->AnchorTransform(
														anchorX, anchorY, anchorZ, anchorYPitch, anchorZYaw, anchorXRoll
														, destX, destY, destZ, destYPitch, destZYaw, destXRoll
													);

													return;
												}
											}
										}
									}
									else
										if (!_stricmp(arg2, "interp"))
										{
											if (4 <= argc)
											{
												char const* arg3 = args->ArgV(3);

												if (!_stricmp(arg3, "position"))
												{
													if (5 <= argc)
													{
														char const* arg4 = args->ArgV(4);
														CamPath::DoubleInterp value;

														if (CamPath::DoubleInterp_FromString(arg4, value))
														{
															camPath->PositionInterpMethod_set(value);
															return;
														}
													}


													conMessage("%s edit interp position ", args->ArgV(0));
													for (CamPath::DoubleInterp i = CamPath::DI_DEFAULT; i < CamPath::_DI_COUNT; i = (CamPath::DoubleInterp)((int)i + 1))
													{
														conMessage("%s%s", i != CamPath::DI_DEFAULT ? "|" : "", CamPath::DoubleInterp_ToString(i));
													}
													conMessage("\n"
														"Current value: %s\n", CamPath::DoubleInterp_ToString(camPath->PositionInterpMethod_get())
													);
													return;
												}
												else
													if (!_stricmp(arg3, "rotation"))
													{
														if (5 <= argc)
														{
															char const* arg4 = args->ArgV(4);
															CamPath::QuaternionInterp value;

															if (CamPath::QuaternionInterp_FromString(arg4, value))
															{
																camPath->RotationInterpMethod_set(value);
																return;
															}
														}


														conMessage("%s edit interp rotation ", args->ArgV(0));
														for (CamPath::QuaternionInterp i = CamPath::QI_DEFAULT; i < CamPath::_QI_COUNT; i = (CamPath::QuaternionInterp)((int)i + 1))
														{
															conMessage("%s%s", i != CamPath::QI_DEFAULT ? "|" : "", CamPath::QuaternionInterp_ToString(i));
														}
														conMessage("\n"
															"Current value: %s\n", CamPath::QuaternionInterp_ToString(camPath->RotationInterpMethod_get())
														);
														return;
													}
													else
														if (!_stricmp(arg3, "fov"))
														{
															if (5 <= argc)
															{
																char const* arg4 = args->ArgV(4);
																CamPath::DoubleInterp value;

																if (CamPath::DoubleInterp_FromString(arg4, value))
																{
																	camPath->FovInterpMethod_set(value);
																	return;
																}
															}


															conMessage("%sedit interp fov ", args->ArgV(0));
															for (CamPath::DoubleInterp i = CamPath::DI_DEFAULT; i < CamPath::_DI_COUNT; i = (CamPath::DoubleInterp)((int)i + 1))
															{
																conMessage("%s%s", i != CamPath::DI_DEFAULT ? "|" : "", CamPath::DoubleInterp_ToString(i));
															}
															conMessage("\n"
																"Current value: %s\n", CamPath::DoubleInterp_ToString(camPath->FovInterpMethod_get())
															);
															return;
														}
											}

											conMessage(
												"%s edit interp position [...]\n"
												"%s edit interp rotation [...]\n"
												"%s edit interp fov [...]\n"
												, args->ArgV(0)
												, args->ArgV(0)
												, args->ArgV(0)
											);
											return;
										}
			}

			conMessage("%s edit start - Sets current demotime as new start time for the path [or selected keyframes].\n", args->ArgV(0));
			conMessage("%s edit start abs <dValue> - Sets a given floating point value as new start time for the path [or selected keyframes].\n", args->ArgV(0));
			conMessage("%s edit start delta(+|-)<dValue> - Offsets the path [or selected keyframes] by the given <dValue> delta value (Example: \"mirv_campath edit start delta-1.5\" moves the path [or selected keyframes] 1.5 seconds back in time).\n", args->ArgV(0));
			conMessage("%s edit duration <dValue> - set floating point value <dValue> as new duration for the path [or selected keyframes] (in seconds). Please see remarks in HLAE manual.\n", args->ArgV(0));
			conMessage("%s edit position current|(<dX> <dY> <dZ>) - Edit position of the path [or selected keyframes]. The position is applied to the center of the bounding box (\"middle\") of all [or the selected] keyframes, meaning the keyframes are moved relative to that. Current uses the current camera position, otherwise you can give the exact position.\n", args->ArgV(0));
			conMessage("%s edit angles current|(<dPitchY> <dYawZ> <dRollX>) - Edit angles of the path [or selected keyframes]. All keyframes are assigned the same angles. Current uses the current camera angles, otherwise you can give the exact angles.\n", args->ArgV(0));
			conMessage("%s edit fov current|<dFov> - Similar to mirv_campath edit angles, except for field of view (fov).\n", args->ArgV(0));
			conMessage("%s  edit rotate <dPitchY> <dYawZ> <dRollX> - Rotate path [or selected keyframes] around the middle of their bounding box by the given angles in degrees.\n", args->ArgV(0));
			conMessage("%s edit anchor #<anchorId>|(<anchorX > <anchorY> <anchorZ> <anchorPitchY> <anchorYawZ> <anchorRollX>) current|(<destX > <destY> <destZ> <destPitchY> <destYawZ> <destRollX>) - This translates and rotates a path using a given anchor (either a keyframe ID or actual values) and a destination for the anchor (use current for current camera view).\n", args->ArgV(0));
			conMessage("%s edit interp [...] - Edit interpolation properties.\n", args->ArgV(0));
			return;
		}
		else if (!_stricmp("select", subcmd))
		{
			if (3 <= argc)
			{
				const char* cmd2 = args->ArgV(2);

				if (!_stricmp(cmd2, "all"))
				{
					camPath->SelectAll();
					return;
				}
				else
					if (!_stricmp(cmd2, "none"))
					{
						camPath->SelectNone();
						return;
					}
					else
						if (!_stricmp(cmd2, "invert"))
						{
							camPath->SelectInvert();
							return;
						}
						else
						{
							bool bOk = true;

							int idx = 2;
							bool add = false;
							if (!_stricmp(cmd2, "add"))
							{
								add = true;
								++idx;
							}

							bool isFromId = false;
							int fromId = 0;
							bool isFromCurrent = false;
							double fromValue = 0.0;
							if (idx < argc)
							{
								const char* fromArg = args->ArgV(idx);

								if (StringBeginsWith(fromArg, "#"))
								{
									isFromId = true;
									++fromArg;
									fromId = atoi(fromArg);
								}
								else if (!_stricmp(fromArg, "current"))
								{
									fromValue = mirvTime->GetTime() - camPath->GetOffset();
								}
								else if (StringBeginsWith(fromArg, "current+"))
								{
									fromValue = mirvTime->GetTime() - camPath->GetOffset();
									fromArg += strlen("current+");
									fromValue += atof(fromArg);
								}
								else if (StringBeginsWith(fromArg, "current-"))
								{
									fromValue = mirvTime->GetTime() - camPath->GetOffset();
									fromArg += strlen("current-");
									fromValue -= atof(fromArg);
								}
								else
								{
									fromValue = atof(fromArg);
								}

								++idx;
							}
							else
								bOk = false;

							bool isToId = false;
							int toId = 0;
							double toValue = 0.0;
							if (idx < argc)
							{
								const char* toArg = args->ArgV(idx);

								if (StringBeginsWith(toArg, "#"))
								{
									isToId = true;
									++toArg;
									toId = atoi(toArg);
								}
								else if (!_stricmp(toArg, "current"))
								{
									toValue = mirvTime->GetTime() - camPath->GetOffset();
								}
								else if (StringBeginsWith(toArg, "current+"))
								{
									toValue = mirvTime->GetTime() - camPath->GetOffset();
									toArg += strlen("current+");
									toValue += atof(toArg);
								}
								else if (StringBeginsWith(toArg, "current-"))
								{
									toValue = mirvTime->GetTime() - camPath->GetOffset();
									toArg += strlen("current-");
									toValue -= atof(toArg);
								}
								else
								{
									toValue = atof(toArg);
								}
								++idx;
							}
							else
								bOk = false;

							bOk = bOk && idx == argc;

							size_t selected = 0;

							if (bOk)
							{
								if (isFromId && isToId)
								{
									if (!add) camPath->SelectNone();
									selected = camPath->SelectAdd((size_t)fromId, (size_t)toId);
								}
								else
									if (!isFromId && isToId)
									{
										if (!add) camPath->SelectNone();
										selected = camPath->SelectAdd((double)fromValue, (size_t)toId);
									}
									else
										if (!isFromId && !isToId)
										{
											if (!add) camPath->SelectNone();
											selected = camPath->SelectAdd((double)fromValue, (double)toValue);
										}
										else bOk = false;
							}

							if (bOk)
							{
								conMessage ("A total of %u keyframes is selected now.\n", selected);
								if (selected < 1)
									conWarning("WARNING: You have no keyframes selected, thus most operations like mirv_campath clear will think you mean all keyframes (i.e. clear all)!\n", selected);

								return;
							}
						}

			}

			conMessage("%s select all - Select all points.\n", args->ArgV(0));
			conMessage("%s select none - Selects no points.\n", args->ArgV(0));
			conMessage("%s select invert - Invert selection.\n", args->ArgV(0));
			conMessage("%s select [add] #<idBegin> #<idEnd> - Select keyframes starting at id <idBegin> and ending at id <idEnd>. If add is given, then selection is added to the current one.\n", args->ArgV(0));
			conMessage("%s select [add] current[(+|-)<dOfsMin>]|<dMin> #<count> - Select keyframes starting at given time and up to <count> number of keyframes. If add is given, then selection is added to the current one.\n", args->ArgV(0));
			conMessage("%s select [add] current[(+|-)<dOfsMin>]|<dMin> current[(+|-)<dOfsMax>]|<dMax> - Select keyframes between given start time and given end time. If add is given, then selection is added to the current one.\n", args->ArgV(0));
			conMessage("Examples:\n");
			conMessage("%s select current #2 - Select two keyframes starting from current time.\n", args->ArgV(0));
			conMessage("%s select add current #2 - Add two keyframes starting from current time to the current selection.\n", args->ArgV(0));
			conMessage("%s select 64.5 #2 - Select two keyframes starting from time 64.5 seconds.\n", args->ArgV(0));
			conMessage("%s select current-0.5 current+2.5 - Select keyframes between half a second earlier than now and 2.5 seconds later than now.\n", args->ArgV(0));
			conMessage("%s select 128.0 current - Select keyframes between time 128.0 seconds and current time.\n", args->ArgV(0));
			conMessage("%s select add 128.0 current+2.0 - Add keyframes between time 128.0 seconds and 2 seconds later than now to the current selection.\n", args->ArgV(0));
			conMessage("Hint: All time values are in game time (in seconds).\n");
			return;
		}
		else if (0 == _stricmp("offset", subcmd))
		{
			if (3 == argc)
			{
				const char* arg2 = args->ArgV(2);

				if (0 == _stricmp("none", arg2))
				{
					camPath->SetOffset(0);
					return;
				}

				double offset = 0;

				if (StringBeginsWith(arg2, "current#"))
				{
					std::string tmpStr(arg2 + strlen("current#"));

					size_t posChr = tmpStr.find('+');
					if (std::string::npos != posChr)
					{
						offset += atof(tmpStr.c_str() + posChr + 1);
						tmpStr = tmpStr.substr(0, posChr);
					}
					else {
						posChr = tmpStr.find('-');
						if (std::string::npos != posChr)
						{
							offset -= atof(tmpStr.c_str() + posChr + 1);
							tmpStr = tmpStr.substr(0, posChr);
						}
					}

					int keyFrame = atoi(tmpStr.c_str());

					int curFrame = 0;

					for (CamPathIterator it = camPath->GetBegin(); it != camPath->GetEnd(); ++it)
					{
						if (curFrame == keyFrame)
						{
							offset += mirvTime->GetCurTime() - it.GetTime();

							camPath->SetOffset(offset);

							return;
						}

						++curFrame;
					}

					conWarning("AFXERROR: Keyframe %i was not found.\n", keyFrame);
					return;
				}
				if (StringBeginsWith(arg2, "current"))
				{
					std::string tmpStr(arg2 + strlen("current"));

					size_t posChr = tmpStr.find('+');
					if (std::string::npos != posChr)
					{
						offset += atof(tmpStr.c_str() + posChr + 1);
						tmpStr = tmpStr.substr(0, posChr);
					}
					else {
						posChr = tmpStr.find('-');
						if (std::string::npos != posChr)
						{
							offset -= atof(tmpStr.c_str() + posChr + 1);
							tmpStr = tmpStr.substr(0, posChr);
						}
					}

					if (camPath->GetSize() < 1)
					{
						conWarning("AFXERROR: Campath must not be empty.\n");
						return;
					}

					offset += mirvTime->GetCurTime() - camPath->GetLowerBound();

					camPath->SetOffset(offset);

					return;
				}
				else
				{
					offset = camPath->GetOffset();
					offset += atof(arg2);
					camPath->SetOffset(offset);
					return;
				}
			}

			conMessage("%s offset none|current[#<iKeyFrameNr>][(+|-)<fSeconds>]|<fSeconds> - Set an offset for the campath to be used (affects playback and various other operations).\n", args->ArgV(0));
			conMessage("Current value: %f\n", camPath->GetOffset());
			conMessage("Examples:\n");
			conMessage("%s offset current -  makes it start at current time.\n", args->ArgV(0));
			conMessage("%s offset current+2.5 - makes it start at current time + 2.5 seconds.\n", args->ArgV(0));
			conMessage("%s offset current#3 - makes current time being at keyframe 3 time.\n", args->ArgV(0));
			conMessage("%s offset current#3-1 - makes current time being at keyframe 3 time - 1 seconds.\n", args->ArgV(0));
			conMessage("%s offset -1 - offsets it by -1 seconds (so it will start earlier).\n", args->ArgV(0));
			conMessage("%s offset none - disables it\n", args->ArgV(0));
			return;
		}
	}

	conMessage("%s add - Adds current demotime and view as keyframe.\n", args->ArgV(0));
	conMessage("%s enabled 0|1 - Set whether the camera path is active or not. Please note that currently at least 4 points are required to make it active successfully!\n", args->ArgV(0));
	conMessage("%s draw [...] - Controls drawing of the camera path.\n", args->ArgV(0));
	conMessage("%s clear - Removes all [or all selected] keyframes.\n", args->ArgV(0));
	conMessage("%s print - Prints keyframes.\n", args->ArgV(0));
	conMessage("%s remove <id> - Removes a keyframe.\n", args->ArgV(0));
	conMessage("%s load <fileName> - Loads the campath from the file (XML format).\n", args->ArgV(0));
	conMessage("%s save <fileName> - Saves the campath to the file (XML format).\n", args->ArgV(0));
	conMessage("%s edit [...] - Edit properties of the path [or selected keyframes].\n", args->ArgV(0));
	conMessage("%s select [...] - Keyframe selection.\n", args->ArgV(0));
	conMessage("%s offset [...] - Offset campath.\n", args->ArgV(0));
	return;
}

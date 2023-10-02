
#include "MirvSkip.h"

#include <iomanip>
#include <sstream>

void MirvSkip_PrintTimeFormated(double time)
{
    int seconds = (int)time % 60;

    time /= 60;
    int minutes = (int)time % 60;

    time /= 60;
    int hours = (int)time;

    std::ostringstream oss;

    oss << std::setfill('0') << std::setw(2);

    if(hours)
    {
        oss << hours << "h";
    }

    oss << minutes << "m" << seconds << "s";

    advancedfx::Message("%s", oss.str().c_str());
}

void MirvSkip_ConsoleCommand(advancedfx::ICommandArgs * args, IMirvCampath_Time* mirvTime, IMirvSkip_GotoDemoTick * pGotoDemoTick)
{
    int argc = args->ArgC();

    if(2 <= argc)
    {
        char const * arg1 = args->ArgV(1);

        if(!_stricmp(arg1, "tick"))
        {
            int curTick;

            if(!mirvTime->GetCurrentDemoTick(curTick))
            {
                advancedfx::Warning("Error: GetCurrentDemoTick failed!\n");
                return;
            }

            if(3 <= argc)
            {
                char const * arg2 = args->ArgV(2);

                if(!_stricmp(arg2, "to") && 4 <= argc)
                {
                    int targetTick = atoi(args->ArgV(3));

                    pGotoDemoTick->GotoDemoTick(targetTick);

                    return;
                }

                if(3 <= argc)
                {
                    int deltaTicks = atoi(arg2);
                    int targetTick = curTick + deltaTicks;

                    pGotoDemoTick->GotoDemoTick(targetTick);

                    return;
                }
            }

            advancedfx::Message(
                    "mirv_skip tick <iValue> - skip approximately integer value <iValue> ticks (negative values skip back).\n"
                    "mirv_skip tick to <iValue> - go approximately to demo tick <iValue>\n"
                    "Current demo tick: %i\n",
                    curTick
            );
            return;
        }
        else
        if(!_stricmp(arg1, "time"))
        {
            double demoTime;
            if(!mirvTime->GetCurrentDemoTime(demoTime))
            {
                advancedfx::Warning("Error: GetCurrentDemoTime failed!\n");
                return;
            }

            if(3 <= argc)
            {
                char const * arg2 = args->ArgV(2);

                if(!_stricmp(arg2, "to") && 4 <= argc)
                {
                    double targetTime = atof(args->ArgV(3));
                    int targetTick;

                    if(!mirvTime->GetDemoTickFromDemoTime(mirvTime->GetCurTime(), targetTime, targetTick))
                    {
                        advancedfx::Warning("Error: GetDemoTickFromDemoTime failed!\n");
                        return;
                    }

                    pGotoDemoTick->GotoDemoTick(targetTick);

                    return;
                }
                else if (0 == _stricmp(arg2, "toGame") && 4 <= argc)
                {
                    double targetClientTime = atof(args->ArgV(3));
                    int targetTick;

                    if (!mirvTime->GetDemoTickFromClientTime(mirvTime->GetCurTime(), targetClientTime, targetTick))
                    {
                        advancedfx::Warning("Error: GetDemoTickFromClientTime failed!\n");
                        return;
                    }

                    pGotoDemoTick->GotoDemoTick(targetTick);

                    return;
                }

                if(3 <= argc)
                {
                    double deltaTime = atof(arg2);
                    double targetTime = mirvTime->GetCurTime() +deltaTime;
                    int targetTick;

                    if(!mirvTime->GetDemoTickFromClientTime(mirvTime->GetCurTime(), targetTime, targetTick))
                    {
                        advancedfx::Warning("Error: GetDemoTickFromClientTime failed!\n");
                        return;
                    }

                    pGotoDemoTick->GotoDemoTick(targetTick);

                    return;
                }
            }

            advancedfx::Message(
                    "mirv_skip time <dValue> - skip approximately time <dValue> seconds (negative values skip back).\n"
                    "mirv_skip time to <dValue> - go approximately to demo time <dValue> seconds\n"
                    "mirv_skip time toGame <dValue> - go approximately to game (client) time <dValue> seconds\n"
                    "Current demo time in seconds: %f (",
                    demoTime
            );
            MirvSkip_PrintTimeFormated(demoTime);
            advancedfx::Message(")\n");

            return;
        }
    }

    advancedfx::Message(
            "mirv_skip tick [...] - skip demo ticks\n"
            "mirv_skip time [...] - skip demo time\n"
    );
    return;
}

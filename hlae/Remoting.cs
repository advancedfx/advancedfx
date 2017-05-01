using System;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Ipc;

using AfxGui;

namespace hlae {
namespace remoting {


// IHlaeRemote_1 ///////////////////////////////////////////////////////////////

/// <summary>
/// Old HLAE remoting interface, i.e used by COL DemoPlayer.
/// </summary>
/// <remarks>Make sure this is defined in the correct namespaces
/// so that it will resolve to hlae.remoting.IHlaeRemote_1.</remarks>
public interface IHlaeRemote_1
{
    /// <summary>
    /// Indicates if the interface is deprecated (meaning if it will
    /// go away in the future.
    /// </summary>
    /// <returns>True if deprecated, false otherwise.</returns>
	bool IsDeprecated();

    /// <summary>
    /// Retrieves user's custom launch arguments from the launcher options.
    /// </summary>
    /// <returns>Null on error, otherwise customargs set by user.</returns>
	String GetCustomArgs();

    /// <summary>
    /// Launches the engine.
    /// </summary>
    /// <returns>False on error, otherwise true.</returns>
	bool Launch();

    /// <summary>
    /// Launches the engine.
    /// </summary>
    /// <param name="OverrideCustomArgs">
    /// Replaces the user's default CustomArgs, also see GetCustomArgs.
    /// </param>
    /// <returns>False on error, otherwise true.</returns>
	bool LaunchEx(String OverrideCustomArgs);
}


// HlaeRemote_1 ////////////////////////////////////////////////////////////////

public class HlaeRemote_1 : MarshalByRefObject, IHlaeRemote_1
{
    //
    // Public members:

    //
    // Interface implementations:

	bool IHlaeRemote_1.IsDeprecated()
	{
		return false;
	}

	String IHlaeRemote_1.GetCustomArgs()
	{
		return GlobalConfig.Instance.Settings.Launcher.CustomCmdLine;
	}

	bool IHlaeRemote_1.Launch()
	{
		return (this as IHlaeRemote_1).LaunchEx( (this as IHlaeRemote_1).GetCustomArgs() );
	}

	bool IHlaeRemote_1.LaunchEx(String OverrideCustomArgs)
	{
        Func<bool> start = delegate
        {
            // first check if HL.exe is already running:
            CfgLauncher cfg = new CfgLauncher();

            cfg.CopyFrom(GlobalConfig.Instance.Settings.Launcher);

            cfg.CustomCmdLine = OverrideCustomArgs;

            return Launcher.Launch(cfg);
        };

        return (bool)m_MainForm.Invoke(start);
	}

    //
    // Internal members:

	internal HlaeRemote_1(MainForm mainForm)
	{
        m_MainForm = mainForm;
	}

    //
    // Private members:

    MainForm m_MainForm;
};


// HlaeRemoting ////////////////////////////////////////////////////////////////

/// <remarks>This is a Singelton class!</remarks>
class HlaeRemoting : IDisposable
{
    //
    // Public members:

    public HlaeRemoting(MainForm mainForm)
	{
		//
		// Init global scope class that will be accessed by the remote:

        m_HlaeRemote_1 = new HlaeRemote_1(mainForm);
		
		//
		// Start remoting server:
        //
		// based on MS example: http://msdn.microsoft.com/en-us/library/system.runtime.remoting.channels.ipc.ipcchannel(VS.80).aspx

		// Create the channel:
		m_ServerChannel = new IpcChannel("localhost:31337");

		// Register the channel:
        ChannelServices.RegisterChannel(m_ServerChannel, false);

		RemotingServices.Marshal(m_HlaeRemote_1, HLAE_REMOTING_OBJ_URI_HlaeRemote_1, typeof(IHlaeRemote_1));
	}

    public void Dispose()
    {
        if (m_Disposed) return;

        RemotingServices.Disconnect(m_HlaeRemote_1);

        ChannelServices.UnregisterChannel(m_ServerChannel);

        m_Disposed = true;
    }

    //
    // Private members:

    const String HLAE_REMOTING_OBJ_URI_HlaeRemote_1 = "Hlae.Remote.1";

    bool m_Disposed;
	IpcChannel m_ServerChannel;
	HlaeRemote_1 m_HlaeRemote_1;
};

} // namespace remoting
} // namespace hlae

// Program.cs - Half-Life Advanced Effects remoting example

// Last changes:
// 2010-06-29 by dominik.matrixstorm.com
//
// First changes:
// 2008-11-02 by dominik.matrixstorm.com

// Compiling:
//
// This example is meant to be built as C# console appication.
// In order for it to compile you need to add a 
// System.Runtime.Remoting framework reference.


// Description:
//
// This example is made from various MSDN Library samples, for more
// information about .NET remoting see here:
// .NET Remoting
// http://msdn.microsoft.com/en-us/library/72x4h507.aspx


using System;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Ipc;


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


} // namespace remoting {
} // namespace hlae {


namespace HlaeRemote {

// Program /////////////////////////////////////////////////////////////////////

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine(
            "Note: This example assumes the user has made his or her default HLAE Launcher"
            +" settings already (path to game etc.) and that the game has not been launched yet."
        );
        Console.WriteLine( "Launch \"hlae.exe -ipcremote\" and press [ENTER] to continue" );
	    Console.ReadLine();

	    IpcChannel channel = new IpcChannel();
	    ChannelServices.RegisterChannel(channel, false);

	    // Instead of creating a new object, this obtains a reference
	    // to the server's single instance of the ServiceClass object:
	    hlae.remoting.IHlaeRemote_1 myremote = (hlae.remoting.IHlaeRemote_1)Activator.GetObject(
		    typeof(hlae.remoting.IHlaeRemote_1),
		    "ipc://localhost:31337/Hlae.Remote.1"
	    );

	    try
	    {
		    // check if the interface is current or if it is deprecated (will be removed or replaced soon):
		    if(myremote.IsDeprecated())
		    {
			    Console.WriteLine("WARNING: This program uses a deprecated interface, please tell the author to update to the new interface version.");
		    }

		    // get the users customargs:
		    String customArgs = myremote.GetCustomArgs();

		    Console.WriteLine("Users current CustomArgs are: {0}", customArgs); 

		    // Let's append s.th. to the user's custormargs:
		    // For this example we will append s.th. that we'll be able to read in the console later.
		    // Please note: some HLAE hooks might not yet be in place when those commands get executed!
		    customArgs += " +echo HelloWorldFromRemoting";

		    Console.WriteLine( "Launching with overriden new CustomArgs: {0}", customArgs); 

		    // Launche the game:
		    if(!myremote.LaunchEx(customArgs))
		    {
			    Console.WriteLine("ERROR: HLAE Failed launching.");
		    } else {
			    Console.WriteLine("HLAE didn't report any problem when launching.");
		    }

		
	    }
	    catch (Exception ex)
	    {
		    Console.WriteLine( String.Format("Exception of type: {0} occurred.", ex.ToString()) );
		    Console.WriteLine( "Details: {0}", ex.Message );
	    }
    }
}

} // namespace HlaeRemote {

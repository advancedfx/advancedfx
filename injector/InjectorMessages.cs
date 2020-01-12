using System;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace advancedfx
{
    namespace injector
    {
        namespace interop
        {
            class Formatter : IFormatter
            {
                internal Formatter()
                {
                    formatter.Binder = new Binder();
                }

                ISurrogateSelector IFormatter.SurrogateSelector { get => formatter.SurrogateSelector; set => formatter.SurrogateSelector = value; }
                SerializationBinder IFormatter.Binder { get => formatter.Binder; set => formatter.Binder = value; }
                StreamingContext IFormatter.Context { get => formatter.Context; set => formatter.Context = value; }

                object IFormatter.Deserialize(Stream serializationStream)
                {
                    return formatter.Deserialize(serializationStream);
                }

                void IFormatter.Serialize(Stream serializationStream, object graph)
                {
                    formatter.Serialize(serializationStream, graph);
                }

                BinaryFormatter formatter = new BinaryFormatter();

                class Binder : System.Runtime.Serialization.SerializationBinder
                {
                    public override Type BindToType(string assemblyName, string typeName)
                    {
                        // Define the new type to bind to
                        Type typeToDeserialize = null;

                        // Get the current assembly
                        string currentAssembly = System.Reflection.Assembly.GetExecutingAssembly().FullName;

                        // Create the new type and return it
                        typeToDeserialize = Type.GetType(string.Format("{0}, {1}", typeName, currentAssembly));

                        return typeToDeserialize;
                    }
                }
            }

            [Serializable]
            public class Message
            {
            }

            //
            // Client messages

            [Serializable]
            public class ClientMessage : Message
            {

            }

            [Serializable]
            public class InjectMessage : ClientMessage
            {
                public UInt32 ProcessId;
                public String DllPath;
            }

            //
            // Client responses:

            [Serializable]
            public class ClientResponse : ClientMessage
            {

            }

            [Serializable]
            public class ContinueWaiting : ClientResponse
            {
                public Boolean Response;
            }

            //
            // Process messages:

            [Serializable]
            public class ProcessMessage : Message
            {

            }

            [Serializable]
            public class Error : ProcessMessage
            {

            }

            [Serializable]
            public class ExceptionError : Error
            {
                public String ExceptionText;
            }

            [Serializable]
            public class WinApiError : Error
            {
                public Int32 GetLastError;
            }

            [Serializable]
            public class OpenProcessError : WinApiError
            {

            }

            [Serializable]
            public class VirtualAllocExError : WinApiError
            {

            }

            [Serializable]
            public class VirtualAllocExArgDllDirError : VirtualAllocExError
            {

            }

            [Serializable]
            public class VirtualAllocExArgDllFilePathError : VirtualAllocExError
            {

            }

            [Serializable]
            public class GetImageError : Error
            {

            }

            [Serializable]
            public class VirtualAllocExImageError : VirtualAllocExError
            {

            }

            [Serializable]
            public class WriteProcessMemoryError : WinApiError
            {

            }

            [Serializable]
            public class WriteProcessMemoryArgDllDirError : WriteProcessMemoryError
            {

            }

            [Serializable]
            public class WriteProcessMemoryArgDllFilePathError : WriteProcessMemoryError
            {

            }

            [Serializable]
            public class WriteProcessMemoryImageError : WriteProcessMemoryError
            {

            }

            [Serializable]
            public class FlushInstructionCacheError : WinApiError
            {

            }

            [Serializable]
            public class CreateRemoteThreadError : WinApiError
            {

            }

            [Serializable]
            public class ContinueWaitingQuestion : ProcessMessage
            {

            }

            [Serializable]
            public class TerminateThreadError : WinApiError
            {

            }

            [Serializable]
            public class GetExitCodeThreadError : WinApiError
            {

            }

            [Serializable]
            public class ExitCodeError : Error
            {

            }

            [Serializable]
            public class InvalidExitCodeError : Error
            {

            }

            [Serializable]
            public class KnownExitCodeError : Error
            {
                public UInt32 ThreadExitCode;
            }

            [Serializable]
            public class CloseHandleError : WinApiError
            {

            }

            [Serializable]
            public class CloseHandleThreadError : CloseHandleError
            {

            }

            [Serializable]
            public class VirtualFreeExError : WinApiError
            {

            }

            [Serializable]
            public class VirtualFreeExImageError : VirtualFreeExError
            {

            }

            [Serializable]
            public class VirtualFreeExArgFilePathError : VirtualFreeExError
            {

            }

            [Serializable]
            public class VirtualFreeExArgDllDirError : VirtualFreeExError
            {

            }

            [Serializable]
            public class CloseHandleProcessError : CloseHandleError
            {

            }

            //
            // Process responses:

            [Serializable]
            public class ProcessResponse : ProcessMessage
            {

            }

            [Serializable]
            public class InjectResponse : ProcessResponse
            {
                public Boolean Response;
            }
        }
    }
}

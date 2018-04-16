using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AfxGui
{
    internal class HlaeErrors : injector.InjectorErrors
    {
        public static new HlaeErrors Instance
        {
            get
            {
                if (null == m_Instance) m_Instance = new HlaeErrors();

                return m_Instance;
            }
        }

        public static advancedfx.AfxError InjectorStartException(string injectorFileName, Exception exception)
        {
            return new advancedfx.AfxError(2000, "Failed to start injector.", "Failed to start injector: " + injectorFileName, "Check that your Anti Virus did not remove it due to a false positive. If so restore it and add an exception for injector / the HLAE folder.", exception);
        }

        public static advancedfx.AfxError LoaderException(Exception exeception)
        {
            if (exeception is advancedfx.AfxError) return exeception as advancedfx.AfxError;

            return new advancedfx.AfxError(2001, "Loader failed.", exeception.ToString(), null, exeception);
        }

        protected HlaeErrors()
        {

        }

        private static HlaeErrors m_Instance;
    }
}

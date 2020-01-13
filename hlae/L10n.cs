using System;
using NGettext;

//
// Usage:
//		L10n._("Hello, World!"); // GetString
//		L10n._n("You have {0} apple.", "You have {0} apples.", count, count); // GetPluralString
//		L10n._p("Context", "Hello, World!"); // GetParticularString
//		L10n._pn("Context", "You have {0} apple.", "You have {0} apples.", count, count); // GetParticularPluralString
//
namespace AfxGui
{
	internal class L10n
	{
		private static readonly ICatalog _Catalog = new Catalog("hlae", System.IO.Path.GetFullPath(System.Windows.Forms.Application.StartupPath).TrimEnd('\\', '/')+"/locales");


		public static string _(string text)
		{
			return _Catalog.GetString(text);
		}

		public static string _(string text, params object[] args)
		{
			return _Catalog.GetString(text, args);
		}

		public static string _n(string text, string pluralText, long n)
		{
			return _Catalog.GetPluralString(text, pluralText, n);
		}

		public static string _n(string text, string pluralText, long n, params object[] args)
		{
			return _Catalog.GetPluralString(text, pluralText, n, args);
		}

		public static string _p(string context, string text)
		{
			return _Catalog.GetParticularString(context, text);
		}

		public static string _p(string context, string text, params object[] args)
		{
			return _Catalog.GetParticularString(context, text, args);
		}

		public static string _pn(string context, string text, string pluralText, long n)
		{
			return _Catalog.GetParticularPluralString(context, text, pluralText, n);
		}

		public static string _pn(string context, string text, string pluralText, long n, params object[] args)
		{
			return _Catalog.GetParticularPluralString(context, text, pluralText, n, args);
		}
	}
}

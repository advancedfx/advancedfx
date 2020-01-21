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

		// // // //

		private static ICatalog _Catalog;

		static L10n()
		{
			string ietfLanguageTagPath = System.IO.Path.Combine(System.Windows.Forms.Application.StartupPath, "locales", System.Globalization.CultureInfo.CurrentUICulture.IetfLanguageTag, "hlae", "messages.mo");
			string twoLetterIsoLanguageNamPath = System.IO.Path.Combine(System.Windows.Forms.Application.StartupPath, "locales", System.Globalization.CultureInfo.CurrentUICulture.TwoLetterISOLanguageName, "hlae", "messages.mo");

			if (System.IO.File.Exists(ietfLanguageTagPath))
				_Catalog = new Catalog(System.IO.File.OpenRead(ietfLanguageTagPath), System.Globalization.CultureInfo.CurrentUICulture);
			else if (System.IO.File.Exists(twoLetterIsoLanguageNamPath))
				_Catalog = new Catalog(System.IO.File.OpenRead(twoLetterIsoLanguageNamPath), System.Globalization.CultureInfo.CurrentUICulture);
			else
				_Catalog = new Catalog();
		}
	}
}

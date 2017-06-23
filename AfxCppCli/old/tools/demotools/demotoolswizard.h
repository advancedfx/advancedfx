#pragma once

using namespace System;

namespace AfxCppCli {
namespace old {
namespace tools {

public ref class DemoToolsWizard
{
public:
	DemoToolsWizard();
	
	/// <returns> true upon success </returns>
	bool ShowDialog(System::Windows::Forms::IWin32Window ^ parentWindow);

	/// <remarks> OutputPath may be changed by the user
	///   during ShowDialog </remarks>
	property String ^ OutputPath
	{
		String ^ get()
		{
			return m_OutputPath;
		}

		void set(String ^ value)
		{
			m_OutputPath = value;
		}
	}

private:
	String ^ m_OutputPath;
};

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {

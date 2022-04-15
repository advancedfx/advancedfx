cmake_minimum_required (VERSION 3.20)

project ("shaderbuilder" VERSION 1.0.0 LANGUAGES CSharp)

include(CSharpUtilities)

add_executable(ShaderBuilder
    "app.config"

   "FxcCompile.cs"
    "Program.cs"
    "SimplePerlExpression.cs"

    "Properties/AssemblyInfo.cs"
)

set(CMAKE_DOTNET_TARGET_FRAMEWORK "net462")

set_property(TARGET ShaderBuilder PROPERTY WIN32_EXECUTABLE TRUE)
set_property(TARGET ShaderBuilder PROPERTY VS_PACKAGE_REFERENCES "SharpDX_4.2.0;SharpDX.D3DCompiler_4.2.0")
set_property(TARGET ShaderBuilder PROPERTY VS_DOTNET_REFERENCES
    "System"
    "System.Core"
    "System.Xml.Linq"
    "System.Data.DataSetExtensions"
    "Microsoft.CSharp"
    "System.Data"
    "System.Deployment"
    "System.Drawing"
    "System.Windows.Forms"
    "System.Xml")

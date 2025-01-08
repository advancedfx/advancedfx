cmake_minimum_required (VERSION 3.24)

project ("AfxHookSource2Snippets")

set(AFXHOOKSOURCE2SNIPPETS_NAMES
    mirv_script_aim
    mirv_script_fov
    mirv_script_point
    mirv_script_voice
    observer-test
    quit-when-demo-is-paused
)

list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_NAMES APPEND ".ts" OUTPUT_VARIABLE AFXHOOKSOURCE2SNIPPETS_INPUTS)
list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_INPUTS PREPEND "${AfxHookSource2Snippets_SOURCE_DIR}/" OUTPUT_VARIABLE AFXHOOKSOURCE2SNIPPETS_INPUTS)
list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_INPUTS PREPEND "\"" OUTPUT_VARIABLE AFXHOOKSOURCE2SNIPPETS_TSC_INPUTS)
list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_TSC_INPUTS APPEND "\"")
list(JOIN AFXHOOKSOURCE2SNIPPETS_TSC_INPUTS "," AFXHOOKSOURCE2SNIPPETS_TSC_INPUTS)

file(WRITE "${AfxHookSource2Snippets_BINARY_DIR}/tsconfig.snippets.json" "{
	\"extends\": \"${AfxHookSource2Snippets_SOURCE_DIR}/../../tsconfig.scripts.json\",
	\"compilerOptions\": {
		\"outDir\": \"${AfxHookSource2Snippets_BINARY_DIR}/dist\",
		\"module\": \"ESNext\",
		\"moduleResolution\": \"Bundler\"
	},
	\"include\": [\"${AfxHookSource2Snippets_SOURCE_DIR}/../types/*\",${AFXHOOKSOURCE2SNIPPETS_TSC_INPUTS}]
}")


list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_NAMES APPEND ".js" OUTPUT_VARIABLE AFXHOOKSOURCE2SNIPPETS_OUTPUTS)
list(TRANSFORM AFXHOOKSOURCE2SNIPPETS_OUTPUTS PREPEND "${AfxHookSource2Snippets_BINARY_DIR}/dist/")

SET(AFXHOOKSOURCE2SNIPPETS_OUTPUTS ${AFXHOOKSOURCE2SNIPPETS_OUTPUTS} PARENT_SCOPE)

add_custom_command(
    OUTPUT ${AFXHOOKSOURCE2SNIPPETS_OUTPUTS}
    COMMAND npm install
    COMMAND npm run tsc -- --project "${AfxHookSource2Snippets_BINARY_DIR}/tsconfig.snippets.json"
    DEPENDS ${AFXHOOKSOURCE2SNIPPETS_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../"
)

add_custom_target(AfxHookSource2Snippets
    DEPENDS ${AFXHOOKSOURCE2SNIPPETS_OUTPUTS}    
)

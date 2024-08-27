module.exports = {
	env: {
		browser: true,
		es2021: true,
		node: true
	},
	extends: [
		'eslint:recommended',
		'plugin:@typescript-eslint/recommended',
		'plugin:prettier/recommended'
	],
	overrides: [
		{
			env: {
				node: true
			},
			files: ['.eslintrc.{js,cjs}'],
			parserOptions: {
				sourceType: 'script'
			}
		}
	],
	parser: '@typescript-eslint/parser',
	parserOptions: {
		ecmaVersion: 'latest',
		sourceType: 'module',
		project: './tsconfig.json'
	},
	plugins: ['@typescript-eslint', 'check-file'],
	rules: {
		'prettier/prettier': [
			'warn',
			{
				endOfLine: 'auto'
			}
		],
		'no-var': 'warn',
		'@typescript-eslint/no-unused-vars': 'warn',
		'@typescript-eslint/ban-ts-comment': 'off',
		'no-shadow': 'off',
		'@typescript-eslint/no-shadow': 'error',
		'@typescript-eslint/naming-convention': [
			'error',
			{
				selector: 'variable',
				filter: '__typename',
				format: null
			},
			{
				selector: 'variable',
				types: ['function'],
				format: ['camelCase', 'PascalCase'],
				leadingUnderscore: 'allow'
			},
			{
				selector: 'variable',
				types: ['boolean', 'number', 'string', 'array'],
				format: ['camelCase', 'UPPER_CASE'],
				leadingUnderscore: 'allow'
			}
		],
		'check-file/filename-naming-convention': [
			'error',
			{
				'**/*.{ts}': 'KEBAB_CASE'
			},
			{
				ignoreMiddleExtensions: true
			}
		],
		'check-file/folder-naming-convention': [
			'error',
			{
				'*/**': 'KEBAB_CASE'
			}
		]
	}
};

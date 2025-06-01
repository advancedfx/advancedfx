import globals from 'globals';
import checkFile from 'eslint-plugin-check-file';
import js from '@eslint/js';
import tseslint from 'typescript-eslint';
import prettier from 'eslint-plugin-prettier/recommended';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { includeIgnoreFile } from '@eslint/compat';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const gitignorePath = path.resolve(__dirname, '.gitignore');

export default [
	includeIgnoreFile(gitignorePath),
	js.configs.recommended,
	...tseslint.configs.recommended,
	prettier,
	{
		ignores: ['.vscode/**', '**/eslint.config.mjs', '**/*.js', '**/*.mjs']
	},
	{
		files: ['**/*.ts', '**/*.mts'],
		plugins: {
			'check-file': checkFile
		},

		languageOptions: {
			parser: tseslint.parser,
			parserOptions: { project: path.join(__dirname, 'tsconfig.json') },
			ecmaVersion: 'latest',
			sourceType: 'module',
			globals: { ...globals.browser, ...globals.node }
		},

		rules: {
			'prettier/prettier': [
				'warn',
				{
					endOfLine: 'auto'
				}
			],

			'no-var': 'warn',
			'no-empty': 'warn',
			'no-shadow': 'off',
			'no-unused-vars': 'off',
			'no-unused-expressions': 'off',

			'@typescript-eslint/no-unused-vars': ['warn', { args: 'none', caughtErrors: 'none' }],
			'@typescript-eslint/no-unused-expressions': [
				'warn',
				{ allowTernary: true, allowShortCircuit: true }
			],
			'@typescript-eslint/ban-ts-comment': 'off',
			'@typescript-eslint/no-shadow': [
				'error',
				{
					ignoreTypeValueShadow: true,
					ignoreFunctionTypeParameterNameValueShadow: true,
					allow: ['T']
				}
			],
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
					'**/*.{ts,mts}': 'KEBAB_CASE'
				},
				{
					ignoreMiddleExtensions: true
				}
			],
			'check-file/folder-naming-convention': [
				'error',
				{
					'*/**/!(examples)': 'KEBAB_CASE'
				}
			]
		}
	},
	{
		files: ['src/snippets/mirv_script_*.ts'],
		rules: {
			'check-file/filename-naming-convention': [
				'error',
				{
					'**/*.{ts,mts}': 'SNAKE_CASE'
				},
				{
					ignoreMiddleExtensions: true
				}
			]
		}
	}
];

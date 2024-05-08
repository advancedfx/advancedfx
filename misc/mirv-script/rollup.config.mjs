import commonjs from '@rollup/plugin-commonjs';
import typescript from '@rollup/plugin-typescript';

export default {
	input: 'src/index.ts',
	output: {
		file: 'dist/mirv.js',
		format: 'cjs'
	},
	plugins: [typescript(), commonjs()]
};

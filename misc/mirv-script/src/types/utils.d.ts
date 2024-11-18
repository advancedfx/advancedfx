/* eslint-disable @typescript-eslint/no-explicit-any */
interface String {
	dedent(): string;
}
interface BigInt {
	toJSON(): string;
}
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON/parse#browser_compatibility
interface JSON {
	parse(
		text: string,
		reviver?: (this: any, key: string, value: any, context: { source: any }) => any
	): any;
}

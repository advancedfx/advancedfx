// Purpose:
// Define reusable wrapper for convenient subcommands declaration.
//
// See usage example in index.ts in 3-command-snippet folder.

type SubCommandType = 'int' | 'float' | 'string' | 'boolean' | 'none' | 'multi';

type SubCommandValue<T extends SubCommandType> = T extends 'string'
	? string
	: T extends 'int' | 'float'
		? number
		: T extends 'boolean'
			? boolean
			: T extends 'multi'
				? string[]
				: T extends 'none'
					? undefined
					: never;

type SubCommandPlaceholder<T extends SubCommandType> = T extends 'multi'
	? string[]
	: T extends 'none'
		? undefined
		: string;

interface BaseSubCommandConfig<T extends SubCommandType> {
	type: T;
	path: string[];
	description: string;
	placeholder: SubCommandPlaceholder<T>;
	initialValue: SubCommandValue<T>;
	onSet?: (value: SubCommandValue<T>) => void;
}

interface SingleValueConfig<T extends 'int' | 'float' | 'string' | 'boolean'>
	extends BaseSubCommandConfig<T> {
	type: T;
}

interface NoArgsConfig<T extends 'none'> extends BaseSubCommandConfig<T> {
	type: T;
}

interface MultiValueConfig<T extends 'multi'> extends BaseSubCommandConfig<T> {
	type: T;
	argsCount: number;
}

type SubCommandConfig<T extends SubCommandType> = T extends 'int' | 'float' | 'string' | 'boolean'
	? SingleValueConfig<T>
	: T extends 'none'
		? NoArgsConfig<T>
		: T extends 'multi'
			? MultiValueConfig<T>
			: never;

export class SubCommand<T extends SubCommandType> {
	readonly type: T;
	readonly path: readonly string[];
	readonly description: string;
	readonly placeholder: SubCommandPlaceholder<T>;
	readonly argsCount: number;
	private _value: SubCommandValue<T>;
	private readonly onSet?: (value: SubCommandValue<T>) => void;

	constructor(config: SubCommandConfig<T>) {
		this.type = config.type as T;
		this.path = [...config.path];
		this.description = config.description;
		this.placeholder = config.placeholder as SubCommandPlaceholder<T>;
		this.argsCount = config.type === 'multi' ? config.argsCount : 1;
		this._value = config.initialValue as SubCommandValue<T>;
		this.onSet = config.onSet as (value: SubCommandValue<T>) => void | undefined;
	}

	get value() {
		return this._value;
	}

	private parse(raw?: string) {
		if (!raw) return null;
		switch (this.type) {
			case 'int': {
				const i = parseInt(raw, 10);
				return isNaN(i) ? null : (i as SubCommandValue<T>);
			}
			case 'float': {
				const f = parseFloat(raw);
				return isNaN(f) ? null : (f as SubCommandValue<T>);
			}
			case 'boolean':
				if (raw === '1') return true as SubCommandValue<T>;
				if (raw === '0') return false as SubCommandValue<T>;
				return null;
			case 'string':
				return raw as SubCommandValue<T>;
			default:
				return null;
		}
	}

	setValue(args: string[]): boolean {
		let parsed: SubCommandValue<T> | null = null;

		switch (this.type) {
			case 'none':
				parsed = undefined as SubCommandValue<T>;
				break;
			case 'multi':
				parsed = args as SubCommandValue<T>;
				break;
			default:
				parsed = this.parse(args[0]);
		}

		if (parsed === null) return false;

		this._value = parsed;
		this.onSet?.(parsed);

		return true;
	}

	getUsage(base: string) {
		if (this.type === 'none') return `${base} ${this.path.join(' ')} - ${this.description}`;

		if (this.type === 'multi')
			return `${base} ${this.path.join(' ')} ${(this.placeholder as string[]).map((v) => `<${v}>`).join(' ')} - ${this.description}`;

		return `${base} ${this.path.join(' ')} <${this.placeholder}> - ${this.description}`;
	}

	getFullUsage(base: string) {
		const value = Array.isArray(this._value)
			? `Current values: ${this._value.join(', ')}`
			: `Current value: ${this._value}`;

		return `Usage:\n${this.getUsage(base)}\n${value}`;
	}
}

export function subCommandsCallback<T extends SubCommandType>(
	subCommands: SubCommand<T>[],
	args: AdvancedfxConCommandArgs
) {
	const argC = args.argC();
	const arg0 = args.argV(0);
	const subArgC = argC - 1;

	const fullUsage = subCommands.map((cmd) => cmd.getUsage(arg0)).join('\n');

	if (argC < 2) {
		console.log(fullUsage);
		return;
	}

	const rawArgs: string[] = [];
	for (let i = 1; i <= subArgC; i++) {
		rawArgs.push(args.argV(i));
	}

	const fullMatches = subCommands.filter((cmd) => {
		if (subArgC < cmd.path.length) return false;
		return cmd.path.every((p, i) => p === rawArgs[i]);
	});

	if (fullMatches.length > 0) {
		const best = fullMatches.reduce((a, b) => (b.path.length > a.path.length ? b : a));

		if (best.type === 'none') {
			best.setValue([]);
			return;
		} else if (subArgC === best.path.length + best.argsCount) {
			if (best.type === 'multi') {
				if (best.setValue(rawArgs.slice(best.path.length))) return;
			} else {
				const raw = rawArgs[best.path.length];
				if (best.setValue([raw])) return;
			}
		}

		console.log(best.getFullUsage(arg0));
		return;
	}

	const partial = subCommands.filter((cmd) => {
		if (subArgC >= cmd.path.length) return false;
		return cmd.path.slice(0, subArgC).every((seg, i) => seg === rawArgs[i]);
	});

	if (partial.length > 0) {
		const msg = partial.map((cmd) => cmd.getUsage(arg0)).join('\n');
		console.log(msg);
		return;
	}

	console.log(fullUsage);
}

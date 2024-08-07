declare class AdvancedfxConCommandArgs {
	argC(): number;
	argV(index: number): string;
}

declare namespace AdvancedfxConCommand {
	type OnCallback = (args: AdvancedfxConCommandArgs) => void;
}

declare class AdvancedfxConCommand {
	constructor(callback: AdvancedfxConCommand.OnCallback);

	register(name: string, description: string): void;

	unregister(): void;
}

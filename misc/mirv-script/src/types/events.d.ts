/**
 * Since HLAE 2.190.0.
 */
declare class AdvancedfxEvent<ResultType> {
	/**
	 * @param defaultResult the initial result value.
	 */
	constructor(defaultResult: ResultType);

	/**
	 * Aborts further dispatching of this event.
	 */
	abort(): void;

	/**
	 * The current result.
	 */
	result: ResultType;
}

/**
 * Since HLAE 2.190.0.
 */
declare class AdvancedfxEventListener<EventType, ResultType> {
	constructor(name: string, callback: AdvancedfxEventListener.Callback<EventType, ResultType>);

	readonly name: string;

	readonly callback: AdvancedfxEventListener.Callback<EventType, ResultType>;
}

/**
 * Since HLAE 2.190.0.
 */
declare namespace AdvancedfxEventListener {
	type Callback<EventType, ResultType> = (e: EventType) => ResultType;
}

/**
 * Since HLAE 2.190.0.
 */
declare class AdvancedfxEventSource<EventType, ResultType> {
	/**
	 * Gets all EventListeners optionall filtered by their priortiy and name.
	 * @param name optional, if undefined all names are returned.
	 * @param priority optional, if undefined all priorites are returned.
	 * @returns The result as Map with priorites and an array of event listeners in the order they would be dispatched.
	 */
	get(
		name?: string,
		priority?: number
	): Map<number, [AdvancedfxEventListener<EventType, ResultType>]>;

	/**
	 * Ads a named callback for the given priority to the end of the array, removing any callback with the same priority and name.
	 * @param name Unique name of the callback.
	 * @param callback The callback to be inserted.
	 * @param priority optional, if undefined will be set to 0. The value -1.0 is used by us for mapping deprecated legacy events properties.
	 * @remarks Does not alter the current dispatch queue of an event when used while the event is being dispatched. Use event's abort function to prevent further processing instead.
	 * @returns the removed old callback with same name and priority, undefined otherwise.
	 */
	on(
		name: string,
		callback: AdvancedfxEventListener.Callback<EventType, ResultType>,
		priority?: number
	): AdvancedfxEventListener.Callback<EventType, ResultType> | undefined;

	/**
	 * Removes the callback with the given name and priority if it exists.
	 * @param name Unique name of the callback.
	 * @param priority optional, if undefined will be set to 0. The value -1.0 is used by us for mapping deprecated legacy events properties.
	 * @remarks Does not alter the current dispatch queue of an event when used while the event is being dispatched. Use event's abort function to prevent further processing instead.
	 * @returns the removed old callback with same name and priority, undefined otherwise.
	 */
	off(
		name: string,
		priority?: number
	): AdvancedfxEventListener.Callback<EventType, ResultType> | undefined;
}

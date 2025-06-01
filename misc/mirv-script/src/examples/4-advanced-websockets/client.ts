// This is part of 4-advanced-websockets example.
// This file is meant to be run in node.
//
// See also server.ts and index.mts
//
// Purpose:
// Simple CLI app to send events to server/HLAE.

import * as readline from 'readline';
import { SimpleWebSocket } from 'simple-websockets';
import { MirvEventsMap } from './events';

const HOST = 'localhost';
const PORT = 31337;
const PATH = 'mirv';

class MirvClient {
	private socket: SimpleWebSocket<MirvEventsMap> | null = null;
	private rl: readline.Interface;

	constructor() {
		this.rl = readline.createInterface({
			input: process.stdin,
			output: process.stdout,
			prompt: 'mirv > '
		});

		this.rl.on('line', async (input) => {
			this.rl.pause();
			await this.handleCommand(input.trim());
			this.rl.resume();
			this.rl.prompt();
		});

		this.rl.on('close', () => {
			process.exit(0);
		});
	}

	private formatTable(rows: string[][], delim: string) {
		const colCount = Math.max(...rows.map((row) => row.length));
		const widths = Array(colCount).fill(0);

		for (const row of rows) {
			for (let i = 0; i < colCount; i++) {
				const cell = row[i] ?? '';
				widths[i] = Math.max(widths[i], cell.length);
			}
		}

		return rows
			.map((row) =>
				row
					.map((cell, i) =>
						i === colCount - 1 ? (cell ?? '') : (cell ?? '').padEnd(widths[i], ' ')
					)
					.join(delim)
			)
			.join('\n');
	}

	private async connectToServer(
		host: string,
		port: number,
		path?: string
	): Promise<SimpleWebSocket<MirvEventsMap>> {
		return new Promise((resolve, reject) => {
			const timeout = setTimeout(() => reject('Connection timeout'), 10_000);
			let address = path ? `ws://${host}:${port}/${path}` : `ws://${host}:${port}/`;
			address += '?clientId=69';
			const socket = new SimpleWebSocket<MirvEventsMap>(address);

			socket._socket.addEventListener('close', (e: unknown) => {
				console.warn('Connection was closed. Reason:', (e as CloseEvent).reason);
				this.socket = null;
			});

			socket.once('connection', () => {
				clearTimeout(timeout);
				console.log(`Connected to ${address}`);
				resolve(socket);
			});
		});
	}

	private setupSocketEventHandlers(): void {
		if (!this.socket) return;

		this.socket.on('onAddEntity', (entity) => {
			const rows = [
				['idx', 'handle', 'debugName', 'className', 'clientClassName'],
				[
					entity.idx.toString(),
					entity.handle.toString(),
					entity.debugName ?? 'null',
					entity.className,
					entity.clientClassName ?? 'null'
				]
			];
			console.log('[ENTITY ADDED]\n' + this.formatTable(rows, ' | '));
		});

		this.socket.on('onRemoveEntity', (entity) => {
			const rows = [
				['idx', 'handle', 'debugName', 'className', 'clientClassName'],
				[
					entity.idx.toString(),
					entity.handle.toString(),
					entity.debugName ?? 'null',
					entity.className,
					entity.clientClassName ?? 'null'
				]
			];
			console.log('[ENTITY REMOVED]\n' + this.formatTable(rows, ' | '));
		});

		this.socket.on('onGameEvent', (id, name, data) => {
			console.log(`[GAME EVENT]\nid: ${id}\nname: ${name}\ndata:\n`, JSON.parse(data));
		});

		this.socket.on('onCViewRenderSetupView', (data: mirv.OnCViewRenderSetupViewArgs) => {
			console.log('[VIEW RENDER SETUP]\n', data);
		});
	}

	private async handleCommand(input: string) {
		if (!input) return;

		const [command, ...args] = input.split(' ');

		switch (command.toLowerCase()) {
			case 'help':
				this.printHelp();
				break;
			case 'connect':
				await this.connect();
				break;
			case 'disconnect':
				this.disconnect();
				break;
			case 'exec':
				if (args.length === 0) {
					console.log('Usage: exec <command>');
					break;
				}
				if (this.socket) this.socket.send('exec', args.join(' '));
				break;
			case 'echo': {
				if (args.length === 0) {
					console.log('Usage: echo <message>');
					break;
				}
				const msg = args.join(' ');
				if (this.socket) this.socket.send('gameConsoleLog', msg, false);
				break;
			}
			case 'entities':
				await new Promise<void>((resolve) => {
					if (!this.socket) {
						resolve();
						return;
					}
					let ok = false;
					const timeout = setTimeout(() => {
						resolve();
						ok = true;
					}, 10_000);
					this.socket.once('listEntities', (entities) => {
						if (ok) return;
						const rows: string[][] = [
							['idx', 'renderEyeOrigin', 'debugName', 'className', 'clientClassName']
						];
						entities.forEach((entity) => {
							rows.push([
								entity.idx.toString(),
								`[ ${entity.renderEyeOrigin.map((v) => parseInt(v.toString())).join(' , ')} ]`,
								entity.debugName ?? 'null',
								entity.className,
								entity.clientClassName ?? 'null'
							]);
						});

						const msg = this.formatTable(rows, ' | ');
						console.log(msg);
						clearTimeout(timeout);
						resolve();
					});
					this.socket.send('listEntities', []);
				});
				break;
			case 'hook': {
				if (!this.socket) {
					console.log('Not connected to server');
					break;
				}

				if (args.length !== 2) {
					console.log(
						'Usage: hook <hookName> <1|0>\nHooks: onViewSetup, onGameEvent, onAddEntity, onRemoveEntity'
					);
					break;
				}
				const type = args[0].toLowerCase();
				const state = args[1] === '1';

				const eventMap: Record<string, keyof MirvEventsMap> = {
					onviewsetup: 'setOnCViewRenderSetupView',
					ongameevent: 'setGameEvent',
					onaddentity: 'setAddEntity',
					onremoveentity: 'setRemoveEntity'
				};

				const ev = eventMap[type];
				if (!ev) {
					console.log(
						'Invalid hook name.\nUse: onViewSetup, onGameEvent, onAddEntity, onRemoveEntity'
					);
					break;
				}

				this.socket.send(ev, state);
				break;
			}
			case 'status':
				console.log(
					`${this.socket ? `Connected to ws://${HOST}:${PORT}/${PATH}` : 'Disconnected.'}`
				);
				break;
			case 'quit':
				this.rl.close();
				break;
			default:
				console.log("Unknown command. Type 'help' for available commands.");
		}
	}

	private printHelp() {
		const rows = [
			['Available commands:'],
			['help', 'Show this help message'],
			['connect', 'Connect to the server'],
			['disconnect', 'Disconnect from the server'],
			['exec <command>', 'Execute a game command'],
			['echo <message>', 'Send a log message to game console'],
			['entities', 'List all entities'],
			[
				'hook <hookName> <1|0>',
				'Enable/disable hooks transmission (onViewSetup, onGameEvent, onAddEntity, onRemoveEntity)'
			],
			['status', 'Show connection status'],
			['quit', 'Exit the application']
		];
		console.log(this.formatTable(rows, ' - '));
	}

	private async connect() {
		if (this.socket) {
			console.log('Already connected to server');
			return;
		}

		try {
			console.log('Connecting to server...');
			this.socket = await this.connectToServer(HOST, PORT, PATH);
			this.setupSocketEventHandlers();
		} catch (error) {
			console.error('Failed to connect:', error);
		}
	}

	private disconnect() {
		if (!this.socket) {
			console.log('Not connected to server');
			return;
		}

		this.socket._socket.close();
		this.socket = null;
		console.log('Disconnected from server');
	}

	start() {
		console.log('Type "help" to see commands or "connect" to start');
		this.rl.prompt();
	}
}

function main() {
	const client = new MirvClient();
	client.start();
}

main();

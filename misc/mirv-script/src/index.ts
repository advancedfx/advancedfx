import { MirvFlags, main } from './mirv/mirv';

{
	const flags: MirvFlags = {
		gameEvents: true,
		cViewRenderSetupView: false
	};
	main(flags, {
		host: 'localhost',
		port: 31337,
		path: 'mirv'
	});
}

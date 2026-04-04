#!/usr/bin/env -S deno run --allow-all
import $ from "jsr:@david/dax@0.44.2";

async function build(args: string[]) {
	await $`zig build -p . ${args}`.noThrow();
}

if (Deno.args.length == 0) {
	console.log("Expected an argument");
	Deno.exit(0);
}

switch (Deno.args[0]) {
	case "build": await build(Deno.args.slice(1)); break;
	case "run": {
		await build([]);
		const result = await $`./bin/wyrm ${Deno.args.slice(1)}`.noThrow();
		if (result.code === 139) {
			console.error("\nSegmentation fault \x1b[2;3m(make.ts)\x1b[0m");
		} else if (result.code === 138) {
			console.error("\nBus error \x1b[2;3m(make.ts)\x1b[0m");
		}
	} break;
	case "test": {
		await build([]);
		await $`./bin/test ./tests`;
	} break;
	case "clear": {
		await Deno.remove(".zig-cache/", { recursive: true })
	} break;
	default:
		console.log(`Unknown command '${Deno.args[0]}'`);
}
<?php

	const TYPES = ["heavy", "light", "scalar"];

	array_shift($argv);

	$csv = "";

	$mode = $argv[0];
	$arg_str = implode(" ", $argv);
	array_shift($argv);

	switch(file_get_contents("/sys/devices/system/cpu/intel_pstate/avxfreq_enabled")) {
		case 0:
			$mode = "hwp";
			break;
		case 1:
			$mode = $mode ? "manual" : "avxfreq";
	}

	foreach($argv as $i => $arg) {
		$csv .= TYPES[$i % 3] . "/" . $arg . ";";
	}

	$csv[strlen($csv) - 1] = "\n";

	for($i = 0; $i < 1000; $i++) {
		$result = `./avx-staged $arg_str`;
		$result = trim($result);

		foreach(explode("\n", $result) as $line) {
			[, $throughput] = explode("=", $line);
			$csv .= $throughput . ";";
		}

		$csv[strlen($csv) - 1] = "\n";
	}

	file_put_contents("results/" . $mode . "_" . implode("_", $argv) . ".csv", $csv);

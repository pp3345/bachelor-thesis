<?php

	$results = file_get_contents("php://stdin");
	if(!strpos($results, "--success--"))
		exit(1);
	$parsed = preg_match_all("/cpu([0-9]+): ([a-zA-Z0-9_]+): (?:0x)?([0-9a-f]+)(?: ovf=([0-1]))?/", $results, $matches);

	chdir("./results");

	$file_name = "results_" . $argv[1] . ".csv";

	$calculate_per_cpu = [
		"tsc_diff-cycles_tsc" => "=tsc_diff-cycles_tsc", // cycles_tsc only counts unhalted cycles
		"non_stall_cycles" => "=cycles-pmc3",
		"non_stall_ratio" => "=non_stall_cycles/cycles",
		"non_stall_cycles_tsc" => "=non_stall_cycles/base_multiplicator",
		"non_throttle_cycles" => "=cycles-pmc6",
		"non_throttle_ratio" => "=non_throttle_cycles/cycles",
		"non_throttle_cycles_tsc" => "=non_throttle_cycles/base_multiplicator",
		"non_throttle_stall_cycles" => "=non_stall_cycles+non_throttle_cycles",
		"non_throttle_stall_cycles_ratio" => "=non_throttle_stall_cycles/cycles",
		"non_throttle_stall_cycles_tsc" => "=non_throttle_stall_cycles/base_multiplicator",
		"fp512_dp_retired/inst_retired" => "=pmc4/inst_retired",
		"base_multiplicator" => "=cycles/cycles_tsc",
		"frequency" => "=base_multiplicator*3.1", // 3.1 GHz is the base frequency of our CPU
		"lvl0_tsc" => "=pmc0/base_multiplicator",
		"lvl1_tsc" => "=pmc1/base_multiplicator",
		"lvl2_tsc" => "=pmc2/base_multiplicator",
		"stall_tsc" => "=pmc3/base_multiplicator",
		"throttle_tsc" => "=pmc6/base_multiplicator",
		"throughput" => "=inst_retired/cycles",
		"time" => "=cycles_tsc*1/3100000000*1000",
/*		"time_with_throttle" => "=time+throttle_tsc*1/3100000000*1000", // do these make sense?
		"time_with_stall" => "=time+stall_tsc*1/3100000000*1000",
		"time_with_stall_throttle" => "=time+(throttle_tsc+stall_tsc)*1/3100000000*1000",*/
	];

	$calculate_all_cpus = [
		"avg_frequency" => "=AVERAGE(frequency)",
		"max_frequency" => "=MAX(frequency)",
		"min_frequency" => "=MIN(frequency)",
		"avg_time" => "=AVERAGE(time)",
		"max_time" => "=MAX(time)",
		"min_time" => "=MIN(time)",
		"avg_cycles_tsc" => "=AVERAGE(cycles_tsc)",
		"max_cycles_tsc" => "=MAX(cycles_tsc)",
		"min_cycles_tsc" => "=MIN(cycles_tsc)",
		"avg_non_throttle_cycles_ratio" => "=AVERAGE(non_throttle_cycles_ratio)",
		"max_non_throttle_cycles_ratio" => "=MAX(non_throttle_cycles_ratio)",
		"min_non_throttle_cycles_ratio" => "=MIN(non_throttle_cycles_ratio)",
		"avg_non_throttle_stall_cycles_ratio" => "=AVERAGE(non_throttle_stall_cycles_ratio)",
		"max_non_throttle_stall_cycles_ratio" => "=MAX(non_throttle_stall_cycles_ratio)",
		"min_non_throttle_stall_cycles_ratio" => "=MIN(non_throttle_stall_cycles_ratio)",
	];

	$cpus = count(array_unique($matches[1]));

	if(!$cpus) {
		// we probably segfaulted
		exit(1);
	}

	$cpu_metrics_length = count($matches[1]) / $cpus;
	$cpu_metrics_ovf_length = $cpu_metrics_length  + count(array_filter($matches[4], function(string $v): bool {
		return (bool) strlen($v);
	})) / $cpus;
	$cpu_length = $cpu_metrics_ovf_length + count($calculate_per_cpu);

	$key_positions = $key_positions_concat = [];

	$add_key_position = function(string $key) use(&$key_positions): void {
		static $pos = "C";

		if(!isset($key_positions[$key]))
			$key_positions[$key] = [];

		$key_positions[$key][] = $pos;
		$pos++;
	};

	$line = ";;";
	for($i = 0; $i < $cpus; $i++) {
		$line .= "cpu" . $i . ";";
		for($j = 0; $j < $cpu_length - 1; $j++)
			$line .= ";";
	}
	if($cpus > 1)
		$line .= "all_cpus;";
	$line[strlen($line) - 1] = "\n";
	$line .= ";;";

	for($i = 0; $i < $cpus; $i++) {
		foreach(array_slice($matches[2], 0, count($matches[1]) / $cpus) as $j => $key) {
			$line .= $key . ";";
			$add_key_position($key);
			if(strlen($matches[4][$j])) {
				$line .= $key . "_ovf;";
				$add_key_position($key . "_ovf");
			}
		}

		foreach($calculate_per_cpu as $k => $v) {
			$line .= $k . ";";
			$add_key_position($k);
		}
	}

	if($cpus > 1) {
		foreach($calculate_all_cpus as $k => $v) {
			$line .= $k . ";";
		}
	}

	$line[strlen($line) - 1] = "\n";
	if(!file_exists($file_name))
		file_put_contents($file_name, $line);

	$matches[3] = array_map(function(string $v, string $ovf): string {
		$v = hexdec($v);

		if(strlen($ovf))
			$v .= ";$ovf";

		return $v;
	}, $matches[3], $matches[4]);

	$line = ";;";

	uksort($key_positions, function(string $a, string $b): int {
		return (-1) * (strlen($a) <=> strlen($b));
	});

	$key_positions_concat = array_map(function(array $cpu_positions): string {
		return implode(",", array_map(function(string $position): string {
			return 'INDIRECT("' . $position. '" & ROW())';
		}, $cpu_positions));
	}, $key_positions);

	for($i = 0; $i < $cpus; $i++) {
		$line .= implode(";", array_slice($matches[3], $i * $cpu_metrics_length, $cpu_metrics_length));
		$line .= ";";
		foreach($calculate_per_cpu as $_ => $v) {
			foreach($key_positions as $k => $cpu_positions) {
				$v = str_replace($k, 'INDIRECT("' . $cpu_positions[$i] . '" & ROW())', $v);
			}
			$line .= $v . ";";
		}
	}

	if($cpus > 1) {
		foreach($calculate_all_cpus as $_ => $v) {
			foreach($key_positions_concat as $key => $replace) {
				$v = str_replace($key, $replace, $v);
			}
			$line .= $v .";";
		}
	}
	$line[strlen($line) - 1] = "\n";

	file_put_contents($file_name, $line, FILE_APPEND);
 

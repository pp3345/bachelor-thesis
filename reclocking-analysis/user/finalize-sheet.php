<?php

	$file_name = "results_" . $argv[1] . ".csv";

	chdir("./results");

	$lines = explode("\n", file_get_contents($file_name));

	$add = "\n";
	$n_metrics = substr_count($lines[1], ";") - 1;
	$n_values = count($lines) - 3;

	foreach(["min" => "=MIN(###)",
		 "max" => "=MAX(###)",
		 "avg" => "=AVERAGE(###)",
		 "median" => "=MEDIAN(###)",
		 "99th" => "=PERCENTILE(###, 0.99)",
		 "98th" => "=PERCENTILE(###, 0.98)",
		 "stddev" => "=STDEV(###)",
		 "variance" => "=VAR(###)"] as $name => $func) {
		$add .= ";" . $name;
		$pos = "C";
		for($i = 0; $i < $n_metrics; $i++) {
			$add .= ";" . str_replace("###", $pos . "3:" . $pos . (2 + $n_values), $func);
			$pos++;
		}
		$add .= ";" . $name;
		$add .= "\n";
	}

	$add .= "\n";
	$add .= $lines[1];
	$add .= "\n";
	$add .= $lines[0];
	$add .= "\n";
	$add .= ";correlation";
	$add .= substr($lines[1], 1);
	$add .= "\n";

	$cpus = explode(";", $lines[0]);
	$metrics = explode(";", $lines[1]);
	array_shift($cpus);
	array_shift($cpus);
	array_shift($metrics);
	array_shift($metrics);
	$ypos = "C";
	foreach($cpus as $i => $cpu) {
		$add .= $cpu . ";" . $metrics[$i];
		$xpos = "C";
		for($i = 0; $i < $n_metrics; $i++) {
			$add .= ";=IFERROR(CORREL(" . $xpos . "3:" . $xpos . (2 + $n_values) . "," . $ypos . "3:" . $ypos . (2 + $n_values) . "), \"\")";
			$xpos++;
		}
		$add .= "\n";
		$ypos++;
	}
	file_put_contents($file_name, $add, FILE_APPEND);

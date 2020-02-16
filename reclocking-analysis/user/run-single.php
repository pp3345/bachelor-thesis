<?php

	const N = 1000;

	[, $section, $license, $cpus, $pre_throttle, $measure] = $argv;

	$name = " ${section}_l${license}_${cpus}cpus";

	if(strpos($section, "256") !== false)
		$vector_size = 256;
	else if(strpos($section, "512") !== false)
		$vector_size = 512;
	else
		throw new Exception("invalid section");

	if(strpos($section, "dp") !== false)
		$fp_precision = "dp";
	else if(strpos($section, "sp") !== false)
		$fp_precision = "sp";
	else
		$fp_precision = "int";

	switch($measure) {
		case 0:
			$name .= "_downlock";
			break;
		case 1:
			$name .= "_upclock";
			break;
		case 2:
			$name .= "_pre_throttle_throughput";
			break;
		case 3:
			$name .= "_throttle_throughput";
			break;
		case 4:
			$name .= "_non_avx_time";
			break;
	}

	switch($pre_throttle) {
		case 1:
			$name .= "_pre_throttle_avx";
			break;
		case 2:
			$name .= "_pre_throttle_scalar";
			break;
	}

	$name .= "_single";

	for($i = 0; $i < N; $i++) {
		`./run.sh .$section $license $cpus $pre_throttle $measure $vector_size $fp_precision | php collect-result.php $name`;
	}

	`php finalize-sheet.php $name`;

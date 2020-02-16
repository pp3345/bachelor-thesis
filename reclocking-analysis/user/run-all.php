<?php

	$tests = [
		["avx_dp_fma_512", 1],
		["avx_dp_fma_512_unrolled", 1],
		["avx_dp_fma_512_unrolled", 2],
		["avx_sp_fma_512", 1],
		["avx_sp_fma_512_unrolled", 1],
		["avx_sp_fma_512_unrolled", 2],
		["avx_dp_mul_512", 1],
		["avx_dp_mul_512", 2],
		["avx_dp_mul_512_unrolled", 1],
		["avx_dp_mul_512_unrolled", 2],
		["avx_sp_mul_512", 1],
		["avx_sp_mul_512", 2],
		["avx_sp_mul_512_unrolled", 1],
		["avx_sp_mul_512_unrolled", 2],
		["avx_int_mul_512", 1],
		["avx_int_mul_512", 2],
		["avx_int_mul_512_unrolled", 1],
		["avx_int_mul_512_unrolled", 2],
		["avx_int_dw_to_w_512", 1],
		["avx_int_dw_to_w_512_unrolled", 1],
		["avx_int_add_512", 1],
		["avx_int_add_512_unrolled", 1],
		["avx_int_mul_add_512", 1],
		["avx_int_mul_add_512", 2],
		["avx_int_mul_add_512_unrolled", 1],
		["avx_int_mul_add_512_unrolled", 2],

		["avx_dp_fma_256_unrolled", 1],
		["avx_sp_fma_256_unrolled", 1],
		["avx_dp_mul_256", 1],
		["avx_dp_mul_256_unrolled", 1],
		["avx_sp_mul_256", 1],
		["avx_sp_mul_256_unrolled", 1],
		["avx_int_mul_256", 1],
		["avx_int_mul_256_unrolled", 1],
		["avx_int_mul_add_256", 1],
		["avx_int_mul_add_256_unrolled", 1],
	];

	const N = 1000;

	file_put_contents("/proc/sys/kernel/sched_rt_runtime_us", 990000);

	foreach($tests as $test) {
		[$section, $license] = $test;
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

		for($cpus = 1; $cpus <= 4; $cpus++) {
			for($no_turbo = 0; $no_turbo < 2; $no_turbo++) {
				foreach([0, 1, 2, 4] as $measure) {
					for($pre_throttle = 0; $pre_throttle < 3; $pre_throttle++) {
						$name = "${section}_l${license}_${cpus}cpus";

						switch($measure) {
							case 0:
								$name .= "_downclock";
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

						if($no_turbo) {
							$name .= "_no_turbo";
						}

						$N = N;

						if(file_exists("results/results_" . $name . ".csv")) {
							$lines = trim(`cat results/results_$name.csv | wc -l`);

							if($lines > $N + 2) {
								file_put_contents("php://stderr", "=== SKIP " . $name . "\n");
								continue;
							}

							$N -= $lines;
							file_put_contents("php://stderr", "=== CONTINUE " . $N . "\n");
						}

						file_put_contents("php://stderr", "=== START " . $name . "\n");

						`rmmod reclocking-analysis-mod`;
						`modprobe reclocking-analysis-mod`;

						file_put_contents("/sys/devices/system/cpu/intel_pstate/no_turbo", $no_turbo);

						$fails = 0;

						for($i = 0; $i < $N; $i++) {
							`sync`;

							$watchdog = proc_open("php watchdog.php " . ($measure == 4 ? 180 : 30), [], $_);
							exec("./run.sh .$section $license $cpus $pre_throttle $measure $vector_size $fp_precision | php collect-result.php $name", $_, $cmd_result);
							proc_terminate($watchdog);
							$watchdog_status = proc_close($watchdog);

							if($cmd_result || $watchdog_status) {
								file_put_contents("php://stderr", "== ERROR\n");

								`rmmod reclocking-analysis-mod`;
								usleep(2000);
								`modprobe reclocking-analysis-mod`;
								usleep(2000);

								$i--;
								$fails++;

								if($fails >= 15 && $i <= 0) {
									// skip broken tests
									continue 2;
								}
							}
						}

						`php finalize-sheet.php $name`;

						file_put_contents("php://stderr", "=== END " . $name . "\n");
					}
				}
			}
		}
	}

	file_put_contents("/sys/devices/system/cpu/intel_pstate/no_turbo", 0);


<?php

	declare(ticks = 1);

	pcntl_signal(SIGTERM, function(): void {
		exit(0);
	});

	sleep($argv[1]);

	if(`ps H -e -o "s command" | grep -e "^D.*reclocking-analysis"`)
		`./run.sh .avx_dp_fma_512 1 1 0 1 512 dp`;

	sleep(1);

	`killall -w "reclocking-analysis"`;

	exit(1);

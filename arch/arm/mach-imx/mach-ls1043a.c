/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <asm/mach/arch.h>

#include "common.h"

static const char * const ls1043a_dt_compat[] __initconst = {
	"fsl,ls1043a",
	NULL,
};

DT_MACHINE_START(LS1043A, "Freescale LS1043A")
	.smp		= smp_ops(layerscape_smp_ops),
	.dt_compat	= ls1043a_dt_compat,
MACHINE_END

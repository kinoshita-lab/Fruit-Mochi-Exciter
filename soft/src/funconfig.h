/**
 * @file funconfig.h
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2024 Kinoshita Lab. All rights reserved.
 *
 */
#pragma once
#ifndef FUNCONFIG_H
#define FUNCONFIG_H
// Though this should be on by default we can extra force it on.
#define FUNCONF_USE_DEBUGPRINTF 1
// #define FUNCONF_DEBUGPRINTF_TIMEOUT (1<<31) // Optionally, wait for a very
//  very long time on every printf.

#define CH32V003 1
#define FUNCONF_SYSTICK_USE_HCLK 1
#endif // FUNCONFIG_H

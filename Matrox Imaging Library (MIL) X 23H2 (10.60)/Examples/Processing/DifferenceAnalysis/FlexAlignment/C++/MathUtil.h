//*************************************************************************************
//
// Synopsis: Math Util macros for customers application.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once 

#define M_Round(x)   ((long)((x) + ((x) >= 0 ? 0.5 : -0.5)))
#define M_Min(x, y)  (((x) < (y)) ? (x) : (y))
#define M_Max(x, y)  (((x) < (y)) ? (y) : (x))

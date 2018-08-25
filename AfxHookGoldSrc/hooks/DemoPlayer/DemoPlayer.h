#pragma once

struct __declspec(novtable) IDemoPlayer001 abstract
{
	virtual void _Unknown_000() abstract = 0;
	virtual void _Unknown_001() abstract = 0;
	virtual void _Unknown_002() abstract = 0;
	virtual void _Unknown_003() abstract = 0;
	virtual void _Unknown_004() abstract = 0;
	virtual void _Unknown_005() abstract = 0;
	virtual void _Unknown_006() abstract = 0;
	virtual void _Unknown_007() abstract = 0;
	virtual void _Unknown_008() abstract = 0;
	virtual void _Unknown_009() abstract = 0;
	virtual void _Unknown_010() abstract = 0;
	virtual void _Unknown_011() abstract = 0;
	virtual void _Unknown_012() abstract = 0;
	virtual void _Unknown_013() abstract = 0;
	virtual void _Unknown_014() abstract = 0;
	virtual void _Unknown_015() abstract = 0;
	virtual void _Unknown_016() abstract = 0;
	virtual void _Unknown_017() abstract = 0;
	virtual void _Unknown_018() abstract = 0;
	virtual void _Unknown_019() abstract = 0;
	virtual void _Unknown_020() abstract = 0;
	virtual void _Unknown_021() abstract = 0;
	virtual void _Unknown_022() abstract = 0;
	virtual void _Unknown_023() abstract = 0;
	virtual void _Unknown_024() abstract = 0;
	virtual void _Unknown_025() abstract = 0;
	virtual void _Unknown_026() abstract = 0;
	virtual void _Unknown_027() abstract = 0;
	virtual void _Unknown_028() abstract = 0;
	virtual void _Unknown_029() abstract = 0;
	virtual void _Unknown_030() abstract = 0;
	virtual void _Unknown_031() abstract = 0;
	virtual void _Unknown_032() abstract = 0;
	virtual void _Unknown_033() abstract = 0;
	virtual double GetDemoTime() abstract = 0;
	virtual void _Unknown_035() abstract = 0;
	virtual void _Unknown_036() abstract = 0;
	virtual void _Unknown_037() abstract = 0;
	virtual void _Unknown_038() abstract = 0;
	virtual void _Unknown_039() abstract = 0;
	virtual void _Unknown_040() abstract = 0;
	virtual void _Unknown_041() abstract = 0;
	virtual void _Unknown_042() abstract = 0;
	virtual void _Unknown_043() abstract = 0;
	virtual void _Unknown_044() abstract = 0;
	virtual void _Unknown_045() abstract = 0;
	virtual void _Unknown_046() abstract = 0;
};

extern IDemoPlayer001 * g_DemoPlayer;

void Hook_DemoPlayer(void * hModule);

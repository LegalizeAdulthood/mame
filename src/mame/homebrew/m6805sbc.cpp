// license:BSD-3-Clause
// copyright-holders:Richard Thomson

// MAME driver for the Legalize Adulthood! MC146805E2 single-board computer.

#include "emu.h"

#include "cpu/m6805/m68705.h"


namespace {

static constexpr u32 M6805SBC_CPU_CLOCK = 4'000'000; // TODO: replace with documented board oscillator value.

class m6805sbc_state : public driver_device
{
public:
	m6805sbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void m6805sbc(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void m6805sbc_state::mem_map(address_map &map)
{
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000);
}

void m6805sbc_state::m6805sbc(machine_config &config)
{
	M146805E2(config, m_maincpu, M6805SBC_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m6805sbc_state::mem_map);
}

ROM_START( m6805sbc )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m6805sbc.bin", 0x1000, 0x1000, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT        COMPANY                FULLNAME                              FLAGS
COMP( 2026, m6805sbc, 0,      0,      m6805sbc, 0,     m6805sbc_state, empty_init, "Legalize Adulthood!", "MC146805E2 Single Board Computer", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )

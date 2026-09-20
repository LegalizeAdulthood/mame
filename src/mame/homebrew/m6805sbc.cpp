// license:BSD-3-Clause
// copyright-holders:Richard Thomson

// MAME driver for the Legalize Adulthood! MC146805E2 single-board computer.

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6805/m68705.h"
#include "machine/6850acia.h"
#include "machine/clock.h"


namespace {

static constexpr u32 M6805SBC_CPU_CLOCK = 4'000'000; // TODO: replace with documented board oscillator value.
static constexpr u32 M6805SBC_ACIA_CLOCK = 1'843'200; // TODO: replace with documented board serial clock.

class m6805sbc_state : public driver_device
{
public:
	m6805sbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "acia")
	{ }

	void m6805sbc(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
};

void m6805sbc_state::mem_map(address_map &map)
{
	// External SRAM decode from 6805_SBC docs/memory-map.txt and design/New_Address_Decode.dig.
	map(0x0002, 0x0003).ram();
	map(0x0006, 0x0007).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x000a, 0x000f).ram();
	map(0x0080, 0x00ff).ram();
	map(0x0110, 0x07ff).ram();
	map(0x0800, 0x0fff).ram();
	map(0x1000, 0x1fff).rom().region("maincpu", 0x1000);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void m6805sbc_state::m6805sbc(machine_config &config)
{
	M146805E2(config, m_maincpu, M6805SBC_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m6805sbc_state::mem_map);

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	clock_device &acia_clock(CLOCK(config, "acia_clock", M6805SBC_ACIA_CLOCK));
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));
}

ROM_START( m6805sbc )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m6805sbc.bin", 0x1000, 0x1000, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT        COMPANY                FULLNAME                              FLAGS
COMP( 2026, m6805sbc, 0,      0,      m6805sbc, 0,     m6805sbc_state, empty_init, "Legalize Adulthood!", "MC146805E2 Single Board Computer", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )

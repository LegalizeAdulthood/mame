-- license:BSD-3-Clause
-- copyright-holders:Richard Thomson

local result_path = [[@RESULT_PATH@]]
local output = assert(io.open(result_path, "w"))
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local failures = {}

local ACIA_STATUS_CONTROL = 0x0006
local ACIA_DATA = 0x0007

local SR_TDRE = 0x02
local SR_DCD = 0x04
local SR_CTS = 0x08

local function fail(message)
	failures[#failures + 1] = message
end

local function has_bits(value, mask)
	return value % (mask * 2) >= mask
end

local function expect_bit(value, mask, label)
	if not has_bits(value, mask) then
		fail(string.format("ACIA status expected %s bit in %02X", label, value))
	end
end

local function expect_clear(value, mask, label)
	if has_bits(value, mask) then
		fail(string.format("ACIA status did not expect %s bit in %02X", label, value))
	end
end

mem:write_u8(ACIA_STATUS_CONTROL, 0x03)
local reset_status = mem:read_u8(ACIA_STATUS_CONTROL)

if reset_status == 0x03 or reset_status == 0xFF then
	fail(string.format("ACIA status/control readback looks unmapped or RAM-backed: %02X", reset_status))
end

mem:write_u8(ACIA_STATUS_CONTROL, 0x15)
local configured_status = mem:read_u8(ACIA_STATUS_CONTROL)

expect_bit(configured_status, SR_TDRE, "TDRE")
expect_clear(configured_status, SR_DCD, "DCD")
expect_clear(configured_status, SR_CTS, "CTS")

mem:write_u8(ACIA_DATA, 0x55)
local transmit_status = mem:read_u8(ACIA_STATUS_CONTROL)

if transmit_status == 0x55 or transmit_status == 0xFF then
	fail(string.format("ACIA data/status path looks unmapped or RAM-backed: %02X", transmit_status))
end

expect_clear(transmit_status, SR_TDRE, "TDRE after transmit-data write")

if #failures == 0 then
	output:write("PASS\n")
else
	output:write("FAIL\n")
	for _, failure in ipairs(failures) do
		output:write(failure, "\n")
	end
end

output:close()
manager.machine:exit()

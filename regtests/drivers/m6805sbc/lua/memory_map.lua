-- license:BSD-3-Clause
-- copyright-holders:Richard Thomson

local result_path = [[@RESULT_PATH@]]
local output = assert(io.open(result_path, "w"))
local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local failures = {}

local function hex(address)
	return string.format("$%04X", address)
end

local function fail(message)
	failures[#failures + 1] = message
end

local function expect(address, value, label)
	local actual = mem:read_u8(address)
	if actual ~= value then
		fail(string.format("%s %s expected %02X got %02X", label, hex(address), value, actual))
	end
end

local writes = {
	{ 0x0002, 0x12 },
	{ 0x0003, 0x34 },
	{ 0x000A, 0x56 },
	{ 0x000F, 0x78 },
	{ 0x0080, 0x9A },
	{ 0x0110, 0xBC },
	{ 0x0800, 0xDE },
}

for _, write in ipairs(writes) do
	mem:write_u8(write[1], write[2])
end

mem:write_u8(0x1002, 0x5A)
mem:write_u8(0x0004, 0xAA)

for _, write in ipairs(writes) do
	expect(write[1], write[2], "RAM")
end

expect(0x1002, 0xA5, "ROM")

if mem:read_u8(0x0004) == 0xAA then
	fail("Internal DDR address $0004 read back the external RAM test pattern")
end

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

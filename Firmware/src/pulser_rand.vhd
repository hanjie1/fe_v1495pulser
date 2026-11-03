library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pulser_rand is
  port(
    RESET        : in  std_logic;
    PLLCLK       : in  std_logic;
    FREQ         : in  std_logic_vector(23 downto 0);
    PULSER_WIDTH : in  std_logic_vector(7 downto 0);
    OUTPUT       : out std_logic
  );
end entity pulser_rand;

architecture rtl of pulser_rand is

  -- K_FACTOR = round(2^64 / 200,000,000) = 92,233,720,369 (37 bits)
  -- Decomposed into:
  --   K_LOW  = 92,233,720,369 mod 2^18 = 188,977 (18 bits)
  --   K_HIGH = 92,233,720,369 / 2^18   = 351,843 (19 bits)
  constant K_LOW_CONST  : unsigned(17 downto 0) := to_unsigned(188977, 18);
  constant K_HIGH_CONST : unsigned(18 downto 0) := to_unsigned(351843, 19);

  -- Initial PRNG seed (non-zero)
  constant SEED_INIT    : unsigned(63 downto 0) := x"5A5A5A5AA5A5A5A5";

  -- Threshold calculation pipeline signals
  signal FREQ_Q         : unsigned(23 downto 0) := (others => '0');
  signal PROD_LOW       : unsigned(41 downto 0) := (others => '0');
  signal PROD_HIGH      : unsigned(42 downto 0) := (others => '0');
  signal THRESHOLD      : unsigned(63 downto 0) := (others => '0');

  -- 64-bit Xorshift PRNG signals
  signal RNG_STATE      : unsigned(63 downto 0) := SEED_INIT;

  -- Pulse generation signals
  signal GENERATOR_EN   : std_logic := '0';
  signal TRIGGER        : std_logic := '0';
  signal PULSE_ACTIVE   : std_logic := '0';
  signal PULSE_COUNT    : unsigned(7 downto 0) := (others => '0');

begin

  -- Generator enable: disabled when FREQ is 0
  GENERATOR_EN <= '0' when unsigned(FREQ) = to_unsigned(0, 24) else '1';

  -- Multiplier pipeline for threshold calculation:
  -- THRESHOLD = FREQ * 2^64 / 200,000,000 = FREQ * 92,233,720,369
  process(PLLCLK)
    variable v_sum : unsigned(63 downto 0);
  begin
    if rising_edge(PLLCLK) then
      if (RESET = '1') or (GENERATOR_EN = '0') then
        FREQ_Q    <= (others => '0');
        PROD_LOW  <= (others => '0');
        PROD_HIGH <= (others => '0');
        THRESHOLD <= (others => '0');
      else
        -- Stage 1: Register input frequency
        FREQ_Q    <= unsigned(FREQ);

        -- Stage 2: Parallel partial DSP multipliers (24x18 and 24x19)
        PROD_LOW  <= FREQ_Q * K_LOW_CONST;
        PROD_HIGH <= FREQ_Q * K_HIGH_CONST;

        -- Stage 3: Recombine partial products
        v_sum     := (resize(PROD_HIGH, 46) & to_unsigned(0, 18)) + resize(PROD_LOW, 64);
        THRESHOLD <= v_sum;
      end if;
    end if;
  end process;

  -- 64-bit Xorshift PRNG generator:
  -- Advances every PLLCLK cycle when generator is enabled (FREQ /= 0)
  process(PLLCLK)
    variable v_s1 : unsigned(63 downto 0);
    variable v_s2 : unsigned(63 downto 0);
    variable v_s3 : unsigned(63 downto 0);
  begin
    if rising_edge(PLLCLK) then
      if RESET = '1' then
        RNG_STATE <= SEED_INIT;
      elsif GENERATOR_EN = '1' then
        v_s1 := RNG_STATE xor (RNG_STATE sll 13);
        v_s2 := v_s1 xor (v_s1 srl 7);
        v_s3 := v_s2 xor (v_s2 sll 17);
        RNG_STATE <= v_s3;
      end if;
    end if;
  end process;

  -- Random event comparison:
  -- Event triggers when enabled and RNG_STATE < THRESHOLD
  process(PLLCLK)
  begin
    if rising_edge(PLLCLK) then
      if (RESET = '1') or (GENERATOR_EN = '0') then
        TRIGGER <= '0';
      else
        if (THRESHOLD /= to_unsigned(0, 64)) and (RNG_STATE < THRESHOLD) then
          TRIGGER <= '1';
        else
          TRIGGER <= '0';
        end if;
      end if;
    end if;
  end process;

  -- Pulse width controller:
  -- PULSER_WIDTH = 0 produces 1 PLLCLK width (5 ns at 200MHz)
  -- PULSER_WIDTH = N produces N + 1 PLLCLK widths
  process(PLLCLK)
  begin
    if rising_edge(PLLCLK) then
      if (RESET = '1') or (GENERATOR_EN = '0') then
        PULSE_ACTIVE <= '0';
        PULSE_COUNT  <= (others => '0');
        OUTPUT       <= '0';
      else
        if PULSE_ACTIVE = '1' then
          if PULSE_COUNT >= unsigned(PULSER_WIDTH) then
            if TRIGGER = '1' then
              PULSE_ACTIVE <= '1';
              PULSE_COUNT  <= (others => '0');
              OUTPUT       <= '1';
            else
              PULSE_ACTIVE <= '0';
              PULSE_COUNT  <= (others => '0');
              OUTPUT       <= '0';
            end if;
          else
            PULSE_COUNT  <= PULSE_COUNT + 1;
            PULSE_ACTIVE <= '1';
            OUTPUT       <= '1';
          end if;
        elsif TRIGGER = '1' then
          PULSE_ACTIVE <= '1';
          PULSE_COUNT  <= (others => '0');
          OUTPUT       <= '1';
        else
          OUTPUT       <= '0';
        end if;
      end if;
    end if;
  end process;

end architecture rtl;

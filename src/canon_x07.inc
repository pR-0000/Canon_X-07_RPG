; ============================================================
; Canon X-07 – ROM & System Routines (FULL / MEGA INCLUDE)
; ============================================================
; CPU: NSC800 (Z80 compatible core)
;
; Tag legend:
;   [OK]  = validated on real hardware / in practice
;   [DOC] = documented in books / PDFs (not necessarily retested)
;   [ROM] = found in ROM tables / disassembly evidence
;   [??]  = guess / needs confirmation
; ============================================================


; ============================================================
; 0) BASICS / COMMON
; ============================================================

TRUE                EQU $FF
FALSE               EQU $00


; ============================================================
; 1) MEMORY MAP / AREAS
; ============================================================

ROM_BASE            EQU $B000
ROM_END             EQU $FFFF

; System work area used by BASIC/interpreter
RAM_SYS_BASE        EQU $0000
RAM_SYS_END         EQU $0552          ; [DOC] “zone système” ~1363 bytes
; ------------------------------------------------------------
; System work area layout (RAM $0000..$0552)                   [DOC]
; ------------------------------------------------------------
; 0000-00AE : conventionally initialized
; 00AE-00B4 : initialized at power-on
; 00B4-00BE : initialized when BASIC is reset
; 00BE-0552 : general system work area

; --- Reset / interrupt vectors (in RAM, patched to ROM entry points)
SYS_RST00_VEC          EQU $0000   ; bytes: C9 (RET) – XP140F breakpoint stub
SYS_RST08_VEC          EQU $0008   ; JP F52F – compare symbol helper
SYS_RST10_VEC          EQU $0010   ; JP F537 – examine next symbol / test char
SYS_RST18_VEC          EQU $0018   ; JP C9C7 – send T6834 cmd (no params) (RST 18)
SYS_RST20_VEC          EQU $0020   ; JP EE45 – compare DE/HL (RST 20)
SYS_RST28_VEC          EQU $0028   ; JP E88F – output A to current device (RST 28)
SYS_RST30_VEC          EQU $0030   ; JP FC2F – test data type (RST 30)
SYS_RST38_VEC          EQU $0038   ; JP E906 – table address calc (RST 38)

SYS_BREAKFLAG          EQU $002B   ; bit0=BREAK, bit1=low batt, bit3=card batt, bit6=timer stop, bit7=OFF
SYS_INT_CURSOR_BLINK   EQU $002C   ; JP/C9 – cursor blink interrupt stub
SYS_INT_SERIAL         EQU $0034   ; JP C7A3 – serial interrupt B
SYS_INT_T6834          EQU $003C   ; JP C799 – T6834 interrupt A

; --- Console / LCD cursor bookkeeping
SYS_CURSOR_XY          EQU $00B8   ; X at $00B8, Y at $00B9
SYS_LCD_LINE_LEN       EQU $00BA   ; LCD line length (chars)
SYS_SCROLL_FIRST       EQU $00BB
SYS_SCROLL_LAST        EQU $00BC
SYS_SCROLL_COUNT       EQU $00BD

; --- BASIC work pointers (selected)
SYS_TXTTAB             EQU $00B2   ; pointer: start of BASIC program text
SYS_LASTLINE           EQU $01DB   ; last line executed
SYS_STKTOP             EQU $01DD   ; end of stack
SYS_MEMSIZ             EQU $01DF   ; end of BASIC memory
SYS_FRETOP             EQU $0204   ; top of string space
SYS_RAMSTR             EQU $0210   ; start of file RAM area
SYS_RAMEND             EQU $0212   ; end of file RAM area

; --- LCD “image mirror” (character-cell mirror, not pixel MEV screen) [DOC]
SYS_LCD_MIRROR         EQU $0214   ; $0214..$0263

; --- T6834 communication scratch (used by BASIC; you can reuse if careful)
SYS_INTIMAG				EQU $026C	; per system-zone dump
SYS_T6834_NREPLY        EQU $026D   ; number of reply bytes expected
SYS_T6834_REPLYBUF      EQU $026E   ; default reply buffer (1st)
SYS_T6834_REPLYBUF2     EQU $0507   ; second reply buffer

; --- INIT# device table and opened device state
SYS_KBNTAB              EQU $02C5  ; 5 x 8 bytes table (INIT#), saves HL/DE/BC etc
SYS_OPEN_DEVNUM          EQU $02F5  ; current open device number
SYS_OPEN_DRIVER_PTR      EQU $02F6  ; current driver address

; --- Cassette filename buffers
SYS_CAS_NAME_SEARCH      EQU $02FF  ; 6 bytes target filename (CLOAD/CLOAD?)
SYS_CAS_NAME_READ        EQU $0305  ; 6 bytes read from tape (SKIP/FOUND)

; --- Runtime flags (selected)
SYS_RSERRFLAG            EQU $030C
SYS_INPUT_ACTIVE          EQU $030D  ; 1 if INPUT in progress
SYS_DATAFLAG              EQU $030E  ; READ flag
SYS_RUN_CALL_TXTTAB       EQU $030F  ; call to TXTTAB on RUN
SYS_CUR_LINE              EQU $0311  ; current line number
SYS_ERROR_LINE            EQU $0317  ; line number where error occurred
SYS_ONERROR_PTR           EQU $0319  ; ON ERROR handler address
SYS_ONERROR_ACTIVE        EQU $031B  ; $FF if ON ERROR set
SYS_CONT_PTR              EQU $031E  ; CONT address
SYS_ERR_LASTCODE_PTR      EQU $0320  ; pointer to last code if error

; --- Variables / arrays tables
SYS_VARTAB               EQU $0322
SYS_ARRTAB               EQU $0324
SYS_STREND               EQU $0326
SYS_DATA_PTR             EQU $0328  ; DATA pointer after RESTORE n

; MEV (T6834 memory window / secondary RAM)
MEV_BASE            EQU $C000

MEV_KEYMODE         EQU $C00C          ; [DOC] 0=NORMAL 1=KANA 2=GRAPH 3=NUM

MEV_ONSTATE         EQU $C0D6          ; [DOC] bit0=ONSTATE bit6=SLEEP bit7=ALARM(sim)
MEV_ALARM_ENABLE    EQU $C01A          ; [DOC] bit0=1 => alarm enabled

MEV_TIME_BASE       EQU $C040          ; [DOC] time fields every 2 bytes:
                                       ; sec,min,hour,dow,date,month,year
MEV_ALARM_BASE      EQU $C050          ; [DOC] alarm fields similar layout

MEV_CURSOR_FLAG     EQU $C080          ; [DOC] cursor indicator
MEV_USERKEY_FLAG    EQU $C082          ; [DOC] user-defined key indicator
MEV_BUZZER_CNT      EQU $C084          ; [DOC] buzzer counter
MEV_TIMER_CNT       EQU $C092          ; [DOC] power timer counter (15 min etc)
MEV_SCROLL_END      EQU $C096          ; [DOC] end scroll
MEV_SCROLL_START    EQU $C09E          ; [DOC] start scroll

; Keyboard buffer in MEV
MEV_KEYBUF          EQU $CC00          ; [DOC] 256 bytes, ~127 keys stored

; User function keys area (MEV)
MEV_FKEYS           EQU $C800          ; [DOC] 512 bytes (12 F-keys slots)

; User-defined characters (MEV)
MEV_USER_CHARS_A    EQU $CA00          ; [DOC] ASCII 80–9F mapped here
MEV_USER_CHARS_B    EQU $CB00          ; [DOC] ASCII E0–FF mapped here

; Boot/autostart program (MEV)
MEV_BOOT            EQU $CE00          ; [DOC] 512 bytes (START$)

; LCD screen memory (MEV)
MEV_SCREEN          EQU $E000          ; [DOC] 511 bytes E000..E1FE


; ============================================================
; 2) REMOTE CONSOLE / “MODE ESCLAVE”
; ============================================================

ROM_CONSOLE_REMOTE_ON    EQU $EE1F      ; [OK] EXEC &HEE1F : redirect console to COM
ROM_CONSOLE_REMOTE_OFF   EQU $EE33      ; [OK] EXEC &HEE33 : restore local console

; Notes:
; - BASIC serial: often works as 8N2 for simple INPUT/PRINT flows [OK]
; - Fast ASM serial I/O for your loader: works reliably in 7E1 [OK]
; - If stuck remote: send EXEC&HEE33 in 8N2 from PC [OK]


; ============================================================
; 3) LCD / SCREEN ROUTINES
; ============================================================

ROM_LCD_PUTC             EQU $C1BE      ; [OK][DOC] A=char -> print + advance cursor, ctrl codes accepted
ROM_LCD_PUTC_ALT          EQU $C1BA      ; [DOC] “display A at cursor and advance” (variant alias)
ROM_LCD_PUTC_RAW          EQU $C231      ; [DOC] H=X L=Y C=char, cursor immobile, MOD AF,BC,DE
ROM_LCD_CLS               EQU $CE9E      ; [OK][DOC] clear screen (CLS)
ROM_LCD_CURSOR_CTRL       EQU $C0A1      ; [DOC] A=0 show cursor, A=1 hide cursor
ROM_COLOR                EQU $A637      ; [DOC] BASIC/ROM COLOR routine, A=color
ROM_SCREEN_STATE         EQU $AB09      ; [DOC] screen/page state helper, D=active page E=visible page C=mode

; Useful display message routines (often used by BASIC)
ROM_MSG_DISPLAY_SYS       EQU $D5B0      ; [DOC] display message (uses BASIC string area) HL->0/0D term
ROM_MSG_DISPLAY_NOSTR     EQU $FEF7      ; [DOC] display message without using BASIC string area

; “Wait cursor” / busy UI
ROM_WAIT_CURSOR           EQU $F23D      ; [DOC] mode “attente curseur”


; ============================================================
; 4) KEYBOARD ROUTINES
; ============================================================

ROM_KBD_GETCHAR           EQU $C8C5      ; [OK][DOC] blocking, A=key, C=1 if interruption, MOD AF
ROM_KBD_INKEY             EQU $C90A      ; [OK][DOC] non-blocking, A=key, Z=1 if none, MOD AF

; Input line with “?” prompt
ROM_INPUTLINE_QMARK       EQU $EBF2      ; [DOC] input line with “?”; HL -> buffer-1
ROM_INPUTLINE_NOPROMPT    EQU $EBFF      ; [DOC] input line without “?”; HL -> buffer-1

; Buffers / breakflag helpers
ROM_BREAKFLAG_QUERY       EQU $C02D      ; [DOC] interrogates BREAKFLAG, affects Z
ROM_BREAKFLAG_CHECKBAT    EQU $C5A6      ; [DOC] check low battery messages / Z indicates status
ROM_BREAKFLAG_ANY         EQU $E8E8      ; [DOC] Z=1 if any BREAKFLAG bit set

; Power / off
ROM_POWER_OFF             EQU $C5C3      ; [DOC] hard power off (cuts power)
ROM_START_VALIDATE         EQU $C5CD      ; [DOC] A=1 validate startup+off, A=2 invalidate+off

; Timer init
ROM_TIMER_INIT_15MIN      EQU $E327      ; [DOC] init 15-minute power timer, MOD AF,B


; ============================================================
; 5) SOUND / BUZZER
; ============================================================

; Verified in BASIC.ROM with ROM base at $B000:
; - ROM_BEEP at $C2DF is the callable ASM entry used by local music.z80
; - ROM_SPK_CONTROL at $C331 is the low-level routine entered by ROM_BEEP
; - the previous $D2xx/$D3xx hypothesis came from a wrong $C000 base assumption
; - ROM_BEEP is not called like a normal ROM routine in ASM:
;     push <continuation address>
;     IY = note/pitch
;     H  = duration
;     L  = 0
;     call ROM_BEEP
;   The routine consumes the normal CALL return address and resumes at the
;   continuation address that was manually pushed beforehand.
ROM_BEEP                  EQU $C2DF      ; [ROM] ASM entry, special stack convention
ROM_SPK_CONTROL           EQU $C331      ; [ROM] low-level speaker routine, not meant to be called directly


; ============================================================
; 6) PRINTER / PARALLEL / IO
; ============================================================

ROM_LPT_INIT              EQU $CFB7      ; [DOC] init parallel printer (text mode etc), MOD AF,BC,DE,HL
ROM_LPT_LFCR              EQU $CFB0      ; [DOC] send LF CR codes, MOD AF,BC,DE,HL
ROM_LPT_SENDCHAR          EQU $CEF7      ; [DOC] A=byte send, checks BUSY/BREAK, MOD AF,BC,DE,HL
ROM_LPT_SENDCHAR_FAST     EQU $CF11      ; [DOC] A=byte send to parallel access point, MOD AF,B,E

ROM_IO_INIT_ALL           EQU $C39D      ; [DOC] init parallel/serial/opt/printer/cassette, MOD AF,BC,DE
ROM_IO_STATUS_WAIT        EQU $CF7A      ; [DOC] wait BUSY low, tests OFF/BREAK/battery, MOD AF

; Generic BASIC/ROM device layer used by INIT#/PRINT#/INPUT# and reusable from ASM
ROM_DEV_OPEN             EQU $E6A8      ; [DOC] open/init device slot
                                          ;   In:  DE = device descriptor table (DEV_*_TABLE)
                                          ;        IY = slot in SYS_KBNTAB ($02C5 + 8*n)
                                          ;        A  = device/file index (usually 0 for #1 slot example code)
                                          ;        For COM only: B = ACIA mode letter, IX = baud divisor/value
                                          ;   Ex: DE=DEV_COM_TABLE, IY=SYS_KBNTAB, A=0, B="B", IX=&1200
                                          ;   Used in book examples for CASI/CASO/COM/GPR/KBD.

ROM_DEV_ASSIGN           EQU $E827      ; [DOC] assign current input/output device from opened slot
                                          ;   In: DE = slot base in SYS_KBNTAB ($02C5, $02CD, ...)
                                          ;   Selects which previously opened device is the current target
                                          ;   for subsequent generic ROM I/O calls such as E88F/E8D4.

ROM_DEV_GETC             EQU $E8D4      ; [DOC] read one byte/char from current assigned device
                                          ;   Out: A = byte received
                                          ;   Notes: book examples use it for CASI and COM; for keyboard/serial
                                          ;   it only returns when a byte is available. Includes ABORT handling.
                                          ;   For cassette input, documentation notes C and Z set together can
                                          ;   signal an I/O/loading error.

ROM_DEV_PUTC             EQU $E88F      ; [DOC] write one byte/char in A to current assigned device
                                          ;   Same routine used behind RST 28 ($0028).


; ============================================================
; 7) T6834 CONTROLLER (Toshiba 6834 sub-CPU)
; ============================================================
; The T6834 manages: keyboard scanning/buffering, LCD display, user-defined
; characters (UDC), user-defined function keys (UDK), clock/alarm, power
; timer, buzzer/click, and various LCD services.
;
; The NSC800 talks to it through ROM helpers and a 2 KB “MEV” memory window
; mapped at $C000..$C7FF (plus related mapped areas documented below).
;
; ---- ROM helper entry points (NSC800 side) ----
ROM_T6834_CMD             EQU $E428      ; [OK][DOC] A=cmd (no params, no reply). MOD AF,C
ROM_T6834_CMD_PARAM       EQU $C92F      ; [OK][DOC] A=cmd, HL=params, B=param count, C=reply count, DE=reply buf. MOD AF,BC,DE,HL
ROM_T6834_FLUSH           EQU $C0BD      ; [OK][DOC] flush keyboard + subprocessor buffers

ROM_T6834_MEV_READ        EQU $E348      ; [DOC] HL=MEV addr -> A=data. MOD AF,BC,DE
ROM_T6834_MEV_WRITE       EQU $E334      ; [OK][DOC] HL=MEV addr, A=data write. MOD AF,BC,DE

ROM_T6834_READY           EQU $C9C0      ; [DOC][??] “wait until T6834 ready” helper (behavior to confirm)

; ---- Calling convention reminder (ROM_T6834_CMD_PARAM / $C92F) ----
; A  = command code
; HL = pointer to parameters bytes (if any)
; B  = number of parameter bytes
; C  = number of reply bytes expected
; DE = pointer to reply buffer (often $026E in BASIC, but you can use your own)
;
; Note: documentation mentions bit7 of A must be set when B=0 (no params).
; In practice many firmwares accept raw codes; safest is:
;   if B==0 : A = A OR $80

; --------------------------------------------------------------------
; T6834 command set (70 total in docs; below are the ones listed in your
; reference text)                                                [DOC]
; --------------------------------------------------------------------
; Naming: T6834_CMD_xxx are raw command numbers (00..46).
; When calling via ROM_T6834_CMD_PARAM, apply OR $80 if B==0.
;
T6834_CMD_TIME_CALL        EQU $01   ; reply 8 bytes: year(2), month, date, dow, hour, min, sec
T6834_CMD_STICK            EQU $02   ; reply 1 byte (cursor). See STICK_* below
T6834_CMD_STRIG            EQU $03   ; reply 1 byte: $00 if F6 pressed else $FF
T6834_CMD_STRIG1           EQU $04   ; reply 1 byte: $00 if SPACE pressed else $FF
T6834_CMD_RAM_READ         EQU $05   ; params: addr_hi, addr_lo ; reply 1 byte: MEV data
T6834_CMD_RAM_WRITE        EQU $06   ; params: data, addr_lo, addr_hi
T6834_CMD_SCROLL_SET       EQU $07   ; params: first_line, last_line
T6834_CMD_SCROLL_EXEC      EQU $08   ; no params: scroll within set range (cursor unchanged)
T6834_CMD_LINE_CLEAR       EQU $09   ; params: line
T6834_CMD_TIME_SET         EQU $0A   ; params: sec,min,hour,dow,date,month,year_lo,year_hi (year = 2 bytes)
T6834_CMD_DOW_CALC         EQU $0B   ; compute/set day-of-week from date
T6834_CMD_ALARM_SET        EQU $0C   ; params like TIME_SET (2nd param “no meaning” per doc)
T6834_CMD_BUZZER_OFF       EQU $0D
T6834_CMD_BUZZER_ON        EQU $0E
T6834_CMD_BITIMAGE_OUT     EQU $0F   ; params: line(0..3) ; reply 120 bytes “bit image”
T6834_CMD_POINT            EQU $10   ; params: X,Y ; reply: $00 if set else $FF
T6834_CMD_PSET             EQU $11   ; params: X,Y
T6834_CMD_PRESET           EQU $12   ; params: X,Y
T6834_CMD_PEOR             EQU $13   ; params: X,Y (toggle)
T6834_CMD_LINE             EQU $14   ; params: X1,Y1,X2,Y2
T6834_CMD_CIRCLE           EQU $15   ; params: X,Y,r
T6834_CMD_UDK_WRITE         EQU $16  ; params: key#, then string (3 label + 38 chars), terminator "00"
T6834_CMD_UDK_READ          EQU $17  ; params: key# ; reply: 255 bytes from UDK table
T6834_CMD_UDK_ON            EQU $18
T6834_CMD_UDK_OFF           EQU $19
T6834_CMD_UDC_WRITE         EQU $1A  ; params: charcode, 8 bytes pattern (6x8 in 8 bytes)
T6834_CMD_UDC_READ          EQU $1B  ; params: charcode ; reply: 8 bytes
T6834_CMD_UDC_INT           EQU $1C  ; “returns to UDC” (wording from doc)
T6834_CMD_SP_WRITE          EQU $1D  ; write start-program (<=511 bytes) with terminator "00"
T6834_CMD_SP_WRITE_CONT     EQU $1E
T6834_CMD_SP_ON             EQU $1F  ; execute start-program
T6834_CMD_SP_OFF            EQU $20
T6834_CMD_SP_READ           EQU $21
T6834_CMD_ON_STATE          EQU $22  ; reply bitfield: bit0=OFF/ON, bit6=SLEEP
T6834_CMD_OFF_REQUEST       EQU $23
T6834_CMD_LOCATE            EQU $24  ; params: X,Y,char (char=00 => just move)
T6834_CMD_CURSOR_ON         EQU $25
T6834_CMD_CURSOR_OFF        EQU $26
T6834_CMD_TEST_KEY          EQU $27  ; params: K (2 bytes?) ; reply OR-sample (see keyboard matrix)
T6834_CMD_TEST_CHR          EQU $28  ; params: ASCII (no SHIFT needed). reply $00 if pressed else $FF
T6834_CMD_SEC_RESET         EQU $29
T6834_CMD_DATE_SET          EQU $2A
T6834_CMD_DISPLAY_OFF       EQU $2B  ; stop LCD common signal; MEV screen still readable/writable
T6834_CMD_DISPLAY_ON        EQU $2C
T6834_CMD_KEYBUF_CLEAR      EQU $2D  ; clear key buffer (keeps UDK/SP)
T6834_CMD_CLS               EQU $2E  ; clear LCD
T6834_CMD_HOME              EQU $2F  ; cursor to (0,0)
T6834_CMD_UDK_LABELS_ON     EQU $30  ; show UDK labels line 3
T6834_CMD_UDK_LABELS_OFF    EQU $31
T6834_CMD_REPEAT_ON         EQU $32
T6834_CMD_REPEAT_OFF        EQU $33
T6834_CMD_KANA_ASSIGN       EQU $34
T6834_CMD_UDK_CONT_WRITE    EQU $35
T6834_CMD_ALARM_READ        EQU $36  ; reply format like TIME_CALL
T6834_CMD_BUZZER_ZERO       EQU $37  ; reply $00 if buzzer count is 0 else $FF
T6834_CMD_CLICK_OFF         EQU $38
T6834_CMD_CLICK_ON          EQU $39
T6834_CMD_LOCATE_CLOSE      EQU $3A  ; clamp cursor back into scroll range after LOCATE
T6834_CMD_CPUKEY_OUT_ON     EQU $3B  ; enable key output to CPU
T6834_CMD_CPUKEY_OUT_OFF    EQU $3C
T6834_CMD_SP_AT_POWERON_ON  EQU $3D  ; run start-program on power-on
T6834_CMD_SP_AT_POWERON_OFF EQU $3E
T6834_CMD_REQUEST_SLEEP     EQU $3F  ; enter SLEEP
T6834_CMD_UDK_INIT          EQU $40   ; restore default UDK assignments
T6834_CMD_CHR_WRITE_AT_CUR  EQU $41   ; params: 8 bytes UDC pattern; writes at cursor and advances
T6834_CMD_CHR_READ_AT_CUR   EQU $42   ; reply: 8 bytes at cursor
T6834_CMD_SCANR             EQU $43   ; reply: 8 bits for 6x8 area at cursor
T6834_CMD_SCANL             EQU $44   ; reply: #points left of (X-1,Y)
T6834_CMD_TIME_CHK          EQU $45   ; params: time; reply bits indicate invalid fields
T6834_CMD_ALM_CHK           EQU $46   ; params: alarm; reply bits indicate invalid fields

; ---- STICK return codes (T6834_CMD_STICK / $02) ----            [DOC]
STICK_NONE                EQU $30
STICK_UP                  EQU $31
STICK_RIGHT               EQU $32
STICK_DOWN                EQU $36
STICK_LEFT                EQU $37

; Note: other documents exist with different cursor codes. The above is the
; set from your latest reference text; your earlier non-blocking demo used a
; different mapping. Keep both if you are comparing sources.                [DOC]

; ============================================================
; 8) SERIAL DEVICE TABLES (BASIC device descriptors)
; ============================================================
; From documentation dump:
DEV_RAM_TABLE        EQU $E7CE          ; [DOC]
DEV_CON_TABLE        EQU $E7EA          ; [DOC]
DEV_KBD_TABLE        EQU $E778          ; [DOC] keyboard/LCD
DEV_GPR_TABLE        EQU $E7B2          ; [DOC] graphic printer
DEV_LPT_TABLE        EQU $E7F8          ; [DOC] centronics
DEV_CASI_TABLE       EQU $E787          ; [DOC] cassette in
DEV_CASO_TABLE       EQU $E796          ; [DOC] cassette out
DEV_OPT_TABLE        EQU $E7DC          ; [DOC] optical coupler
DEV_COM_TABLE        EQU $E7A4          ; [DOC] serial port COM
DEV_PRT_TABLE        EQU $E7C0          ; [DOC] serial printer

; ============================================================
; 8.9) HD61L202F – I/O CONTROL CHIP (ports $F0..$F7)
; ============================================================
; [DOC] “Puce de contrôle entrées/sorties” between NSC800 and:
;   - Centronics (parallel) control/status
;   - Serial TX/RX (UART + baud generator)
;   - Buzzer output
;   - Cassette interface
; NSC800 accesses registers via I/O ports $F0..$F7 (OUT/IN).
;
; -------------------------
; Port addresses
; -------------------------
IOP_F0_INTCTRL          EQU $F0           ; R/W: interrupt enables / TRQ flags
IOP_F1_XBTR_XBRR        EQU $F1           ; W: XBTR (to T6834) / R: XBRR (from T6834)
IOP_F2_BAUD_L_FLAGS     EQU $F2           ; W: baud low byte / R: system flags
IOP_F3_BAUD_H           EQU $F3           ; W: baud high nibble (bits 3..0) / R: (unused)
IOP_F4_MODE             EQU $F4           ; R/W: mode bits (MD1/MD0 etc)
IOP_F5_INTRESET         EQU $F5           ; W: interrupt/reset flags (ack)
IOP_F6_UART             EQU $F6           ; W: UART config / R: UART status
IOP_F7_UART_DATA        EQU $F7           ; W: serial TX data / R: serial RX data
;
; -------------------------
; $F0 – interrupt control (WRITE)
; -------------------------
F0_OFFRQ                EQU $80           ; request power off
F0_SETPS                EQU $40           ; power-save (sleep/off state)
F0_BIE                  EQU $20           ; enable baud generator interrupts
F0_ALMIE                EQU $10           ; enable alarm interrupts
F0_XTIE                 EQU $08           ; enable extension-bus interrupts
F0_XRIE                 EQU $04           ; enable serial RX interrupts
F0_TIE                  EQU $02           ; enable serial TX interrupts
F0_RIE                  EQU $01           ; enable serial “receive” interrupts (see doc)
;
; $F0 – interrupt control (READ)
F0_TRQ1                 EQU $80           ; [DOC] set by T6834: “cannot enter save mode”
F0_TRQ0_BREAK           EQU $40           ; [DOC] set if BREAK key pressed
; other bits reflect enables: BIE/ALMIE/XTIE/XRIE/TIE/RIE
;
; -------------------------
; $F2 – baud generator (WRITE) / flags (READ)
; -------------------------
; Baud generator is 12-bit:
;   - low 8 bits  -> $F2
;   - high 4 bits -> $F3 bits 3..0
;
F2R_CBSY                EQU $80           ; parallel busy
F2R_ALRM_BATLOW         EQU $40           ; [DOC] batteries low if 1 (checked periodically)
F2R_AUX1_RAMSW          EQU $20           ; [DOC] ext switch: 0 when set to RAM
F2R_AUX2_CARDBAT        EQU $10           ; [DOC] memory card battery state
F2R_BG1                 EQU $04           ; baud generator output (timing)
F2R_XTRE                EQU $02           ; [DOC] set when T6834 reads XBTR
F2R_XRRE                EQU $01           ; [DOC] set when T6834 writes XBRR
;
; -------------------------
; $F4 – modes (READ/WRITE)
; -------------------------
F4_SETBC                EQU $80           ; latch baud settings from $F2/$F3 into generator
F4_BRGST                EQU $40           ; baud generator run; stops when reaches 0
F4_CNTR                 EQU $20           ; bit storage to parallel output
F4_LEO                  EQU $10           ; 38.4 kHz to optical coupler
F4_MD1                  EQU $08           ; mode select bit1
F4_MD0                  EQU $04           ; mode select bit0
F4_BZON                 EQU $02           ; buzzer on (raw)
F4_REM                  EQU $01           ; REM line / cassette remote
;
; MD1/MD0 decode (WRITE)
;   0/0: RESET (baud counter stops)
;   0/1: SERIAL
;   1/0: CASSETTE
;   1/1: BUZZER
;
; -------------------------
; $F6 – UART config (WRITE)
; -------------------------
; Bit mapping from doc table (unknown bits omitted):
F6W_IR                  EQU $40           ; init UART
F6W_RTS                 EQU $20           ; RTS control (when 1, RTS pin forced low)
F6W_ER                  EQU $10           ; reset error flags (PE/OE/FE)
F6W_SBRK                EQU $08           ; force BREAK (continuous 0)
F6W_RXE                 EQU $04           ; enable RX
F6W_TXEN                EQU $01           ; enable TX
;
; [DOC] Another line mentions “F6 (TRANSMISSION SERIE) S2/EP/PEN/L1/B2/FX04”.
; This likely refers to additional serial-format settings. Not yet validated in ROM.
;
; $F6 – UART status (READ)
F6R_CTS                 EQU $80           ; CTS input (0 allows TX when TX enabled)
F6R_FE                  EQU $20           ; framing error (missing stop bit)
F6R_OE                  EQU $10           ; overrun error
F6R_PE                  EQU $08           ; parity error
F6R_TXE                 EQU $04           ; TX register empty / error (doc wording)
F6R_RXRDY               EQU $02           ; RX ready
F6R_TXRDY               EQU $01           ; TX ready

; ============================================================
; 9) BASIC KEYWORDS ENTRY POINTS (ROM table)
; ============================================================
; These are useful as “ROM anchors” and for debugging, even if not all are used in ASM.
; Format: KEYWORD  entry address (often interpreter dispatcher)
; [DOC]/[ROM] list (not all validated).

ROM_ABS                EQU $C9DC        ; [DOC]
ROM_BEEP_BASIC          EQU $C2CC        ; [ROM] BASIC BEEP wrapper/parser
ROM_CINT               EQU $CAE0        ; [DOC]
ROM_CSNG               EQU $CB08        ; [DOC]
ROM_CDBL               EQU $CB90        ; [DOC]
ROM_CLS_BASIC          EQU $CE9E        ; [OK][DOC]
ROM_LOCATE_BASIC       EQU $E212        ; [DOC]
ROM_PRINT_BASIC        EQU $EA03        ; [DOC]
ROM_EXEC_BASIC         EQU $CEB8        ; [DOC]
ROM_INKEY_BASIC        EQU $C103        ; [DOC]
ROM_INPUT_BASIC        EQU $F82F        ; [DOC]
ROM_PEEK_BASIC         EQU $FC7A        ; [DOC]
ROM_POKE_BASIC         EQU $FC80        ; [DOC]
ROM_RUN_BASIC          EQU $F1B7        ; [DOC]
ROM_NEW_BASIC          EQU $D214        ; [DOC]
ROM_END_BASIC          EQU $D2B0        ; [DOC]
ROM_STOP_BASIC         EQU $D2C0        ; [DOC]
ROM_GOTO_BASIC         EQU $F621        ; [DOC]
ROM_GOSUB_BASIC        EQU $F610        ; [DOC]
ROM_RETURN_BASIC       EQU $F63D        ; [DOC]
ROM_LIST_BASIC         EQU $FE2B        ; [DOC]
ROM_LLIST_BASIC        EQU $FE5B        ; [DOC]
ROM_ERROR_BASIC        EQU $F78C        ; [DOC]
ROM_HEX_INPUT          EQU $FB4F        ; [DOC] “entrée routine hexadécimale”
ROM_COLD_START         EQU $C3C3        ; [DOC] init générale (cold start)
ROM_COPYRIGHT_MSG      EQU $C528        ; [DOC] points to copyright display
ROM_CREATE_SYSTEM_Q    EQU $C4D1        ; [DOC] “CREATE SYSTEM ?” (card wipe helper)

; Error display helpers
ROM_ERR_SN             EQU $F1AA        ; [DOC] “SN ERROR”
ROM_ERR_THROW          EQU $F1C7        ; [DOC] E=error number -> display error

; Expression evaluators mentioned in doc (handy in tooling)
; ROM_EVAL_INT_SMALL      EQU $F?ES        ; [DOC][??] doc text looks OCR-corrupted (“fesses”)
ROM_EVAL_INT_ALT        EQU $FFCC        ; [DOC] integer eval, result in DE and A


; ============================================================
; 10) CONVERSIONS / MATH / ARITH (DOCUMENTED)
; ============================================================

; --- Type conversions
ROM_CINT_RTN            EQU $CAE0        ; [DOC] float->int (CINT) modifies all regs
ROM_CSNG_RTN            EQU $CB08        ; [DOC] convert to single precision
ROM_CSNG_FROM_INT       EQU $CB1E        ; [DOC]
ROM_CDBL_RTN            EQU $CB90        ; [DOC] convert to double precision
ROM_FPACC_TO_BCDE       EQU $CA26        ; [DOC] transfer BASIC numeric accumulator (single precision) to BC/DE

; --- ASCII -> binary
ROM_ATOI_INT            EQU $F595        ; [DOC] HL->string, DE=result
ROM_ATOF_FLOAT          EQU $BA21        ; [DOC] HL->string, result in accumulator

; --- Binary -> ASCII
ROM_HL_TO_ASCII_PRINT   EQU $BB98        ; [DOC] HL value -> prints ASCII (debug helper)
ROM_AINT_TO_ASCII_BUF   EQU $BE5F        ; [DOC] int in accumulator, HL->buf, B/C > 6
ROM_FLOAT_TO_ASCII      EQU $BBAB        ; [DOC] float formatting flags in A, digits B/C, HL->buf

; --- Integer arithmetic
; ROM_INT_ADD             EQU $CCB?        ; [DOC][??] address OCR in text (addition integer)
ROM_INT_SUB             EQU $CCBB        ; [DOC] HL-DE
ROM_INT_MUL             EQU $CCF7        ; [DOC]
ROM_INT_DIV             EQU $FA79        ; [DOC] DE/HL, quotient in double accumulator

; --- Single precision ops
ROM_SNG_ADD             EQU $CDA9        ; [DOC]
ROM_SNG_SUB             EQU $CDB2        ; [DOC]
ROM_SNG_MUL             EQU $CDB7        ; [DOC]
ROM_SNG_DIV             EQU $CDC2        ; [DOC]

; --- Double precision ops
ROM_DBL_ADD             EQU $B20E        ; [DOC]
ROM_DBL_SUB             EQU $B200        ; [DOC]
ROM_DBL_MUL             EQU $B376        ; [DOC]
ROM_DBL_DIV             EQU $B42F        ; [DOC]

; --- Comparisons
ROM_SNG_CMP             EQU $CA7B        ; [DOC]
ROM_INT_CMP             EQU $CAA5        ; [DOC]
ROM_DBL_CMP             EQU $CAD9        ; [DOC]
ROM_DBL_SGN             EQU $C9FB        ; [DOC]

; --- Math functions
ROM_ABS_RTN             EQU $C9DC        ; [DOC]
ROM_INT_PART            EQU $CC23        ; [DOC] INT()


; ============================================================
; 11) RESTART VECTORS (DOCUMENTED)
; ============================================================

; RST 00: breakpoint for XP-140 monitor card (RET at 0000) [DOC]
; RST 08: compare symbol routine at F52F [DOC]
; RST 10: examine next symbol at F537 [DOC]
; RST 18: T6834 command without params at C937 [DOC]
; RST 20: compare DE with HL at EE45 [DOC]
; RST 28: output A to current device (often LCD) at E88F [DOC]
; RST 30: test type at FC2F [DOC]
; RST 38: calc table addresses at E906 [DOC]

RST18_T6834_NOPARAM      EQU $C937        ; [DOC] (routine behind RST 18)
RST28_DEV_PUTC           EQU $E88F        ; [DOC] (routine behind RST 28)

; Common aliases matching the generic ROM device layer names
ROM_IO_OPEN_DEVICE       EQU ROM_DEV_OPEN
ROM_IO_ASSIGN_DEVICE     EQU ROM_DEV_ASSIGN
ROM_IO_GETCHAR           EQU ROM_DEV_GETC
ROM_IO_PUTCHAR           EQU ROM_DEV_PUTC


; ============================================================
; 12) KEY CODES (CONFIRMED + EXTENDED)
; ============================================================

; --- Control / navigation (validated by your getkey logging) [OK]
KEY_ON_BREAK             EQU $01
KEY_RETURN               EQU $0D
KEY_SPACE                EQU $20

KEY_HOME_CLR             EQU $0B
KEY_INS                  EQU $12
KEY_DEL                  EQU $16

KEY_RIGHT                EQU $1C
KEY_LEFT                 EQU $1D
KEY_UP                   EQU $1E
KEY_DOWN                 EQU $1F

; --- Digits & punctuation are ASCII (examples)
KEY_MINUS                EQU $2D          ; '-'
KEY_COMMA                EQU $2C          ; ','
KEY_DOT                  EQU $2E          ; '.'
KEY_SLASH                EQU $2F          ; '/'
KEY_COLON                EQU $3A          ; ':'
KEY_SEMICOLON            EQU $3B          ; ';'
KEY_AT                   EQU $40          ; '@'
KEY_LBRACKET             EQU $5B          ; '['
KEY_RBRACKET             EQU $5D          ; ']'
KEY_CARET                EQU $5E          ; '^'

; Notes:
; - CTRL alone returns no code, but modifies letters (CTRL+Z = $1A etc) [OK]
; - SHIFT alone returns no code, SHIFT+letters => lowercase ASCII [OK]
; - GRPH modifies letters to extended values (>= $80) [OK]

; ============================================================
; 14) T6834 STICK (NON-BLOCKING ARROWS VIA CONTROLLER)
; ============================================================
; Documented sample:
;   CALL ROM_T6834_FLUSH
;   A=$82 (STICK command #2)
;   BC=$0001 (expect 1 reply byte)
;   DE=BUF
;   CALL ROM_T6834_CMD_PARAM
;   A=(BUF)
;   $30 = none, $33=right, $37=left, $35=down, $31=up   [DOC]
;
; These values differ from GETKEY codes (1C..1F).   [OK]

T6834_STICK_CMD           EQU $82          ; [DOC]
T6834_STICK_NONE          EQU $30          ; [DOC]
T6834_STICK_UP            EQU $31          ; [DOC]
T6834_STICK_RIGHT         EQU $33          ; [DOC]
T6834_STICK_DOWN          EQU $35          ; [DOC]
T6834_STICK_LEFT          EQU $37          ; [DOC]


; ============================================================
; 15) FUNCTION KEYS – ADDRESSES (MEV)
; ============================================================
; Figure 26 addresses (user-defined function keys slots) [DOC]

FKEY_ADDR_F1              EQU $C800
FKEY_ADDR_F2              EQU $C82A
FKEY_ADDR_F3              EQU $C85A
FKEY_ADDR_F4              EQU $C87E
FKEY_ADDR_F5              EQU $C8A8
FKEY_ADDR_F6              EQU $C8D2
FKEY_ADDR_F7              EQU $C900
FKEY_ADDR_F8              EQU $C92A
FKEY_ADDR_F9              EQU $C954
FKEY_ADDR_F10             EQU $C97E
FKEY_ADDR_F11             EQU $C9A8
FKEY_ADDR_F12             EQU $C9D2


; ============================================================
; 16) KEYBOARD MATRIX (DOCUMENTATION – HARDWARE LEVEL)
; ============================================================
; Provided table (Figure 25) – included as reference [DOC]
; Rows S1..S8 with bitmasks, columns K1..K8:
;   SHIFT, CTRL, GRPH, NUM, OFF, ON, etc
; Note: Scanning this directly likely done by T6834 firmware; not a typical use in ASM.
;
; Row masks:
KBD_ROW_S1_MASK            EQU $0001       ; [DOC]
KBD_ROW_S2_MASK            EQU $0002       ; [DOC]
KBD_ROW_S3_MASK            EQU $0004       ; [DOC]
KBD_ROW_S4_MASK            EQU $0008       ; [DOC]
KBD_ROW_S5_MASK            EQU $0010       ; [DOC]
KBD_ROW_S6_MASK            EQU $0020       ; [DOC]
KBD_ROW_S7_MASK            EQU $0040       ; [DOC]
KBD_ROW_S8_MASK            EQU $0080       ; [DOC]
KBD_ROW_BZ_MASK            EQU $0100       ; [DOC]
KBD_ROW_A1_MASK            EQU $0200       ; [DOC]


; ============================================================
; 17) CONTROL CODES ABBREVIATIONS (DOC)
; ============================================================

CC_BELL              EQU $07     ; [DOC] BELL (sonnerie)
CC_BS                EQU $08     ; [DOC] backspace
CC_LF                EQU $0A     ; [DOC] line feed
CC_LU                EQU $0B     ; [DOC] line up (back one line) (note: also HOME/CLR key code differs)
CC_DC1               EQU $11     ; [DOC] device control 1 (text/graphic on X-710)
CC_DC2               EQU $12     ; [DOC] device control 2 (graphic/text on X-710)
CC_SI                EQU $0F     ; [DOC] shift in (external ROM chars on X-710)
CC_SO                EQU $0E     ; [DOC] shift out (internal ROM chars on X-710)


; ============================================================
; END OF FILE
; ============================================================

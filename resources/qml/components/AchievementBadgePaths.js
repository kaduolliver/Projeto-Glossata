// Own geometric drawings created for substep 2.3.3; rights decision pending.
.pragma library
var drawings = {
    "leaf": {
        "color": "#91e3ac",
        "path": "M34 69Q25 32 67 29Q81 63 34 69ZM34 69L63 40M48 53L44 40M48 53L64 55"
    },
    "seed": {
        "color": "#91e3ac",
        "path": "M49 74V49M49 58Q23 57 28 37Q50 34 49 58M50 48Q51 26 75 29Q77 51 50 48"
    },
    "flag": {
        "color": "#91e3ac",
        "path": "M35 77V27M36 29Q50 20 61 30L73 28V54Q61 60 52 49L36 51"
    },
    "compass": {
        "color": "#91e3ac",
        "path": "M50 25A25 25 0 1 0 50 75A25 25 0 1 0 50 25ZM61 36L54 54L36 64L44 45Z"
    },
    "lamp": {
        "color": "#91e3ac",
        "path": "M40 63Q22 43 38 31Q51 21 63 32Q77 45 60 63ZM40 69H60M44 76H56M50 7V16M17 29L23 33M77 32L84 27"
    },
    "eye": {
        "color": "#eac77d",
        "path": "M19 51Q50 18 81 51Q50 82 19 51ZM50 38A13 13 0 1 0 50 64A13 13 0 1 0 50 38ZM50 46V55"
    },
    "sprout": {
        "color": "#eac77d",
        "path": "M50 76V49M50 58Q21 54 23 31Q48 29 50 58M50 49Q53 25 79 28Q76 52 50 49M30 77H70"
    },
    "book": {
        "color": "#eac77d",
        "path": "M50 34Q32 24 22 30V71Q37 64 50 74Q64 64 78 71V30Q67 24 50 34V74M29 41L41 44M59 44L71 41M29 52L41 55M59 55L71 52"
    },
    "laurel": {
        "color": "#eac77d",
        "path": "M46 76Q15 60 29 29M28 60L16 49M28 48L18 35M30 36L24 22M54 76Q85 60 71 29M72 60L84 49M72 48L82 35M70 36L76 22M50 35L54 45L65 46L57 54L59 66L50 60L41 66L43 54L35 46L46 45Z"
    },
    "medal": {
        "color": "#eac77d",
        "path": "M36 25L42 49H58L65 25M50 44A18 18 0 1 0 50 80A18 18 0 1 0 50 44ZM50 51L53 58L61 59L55 64L57 72L50 68L43 72L45 64L39 59L47 58Z"
    },
    "spark": {
        "color": "#eac77d",
        "path": "M50 20L57 42L79 50L57 58L50 80L43 58L21 50L43 42ZM75 20V30M70 25H80"
    },
    "target": {
        "color": "#eac77d",
        "path": "M50 23A27 27 0 1 0 50 77A27 27 0 1 0 50 23ZM50 36A14 14 0 1 0 50 64A14 14 0 1 0 50 36ZM50 50L78 22M66 23H78V35"
    },
    "wing": {
        "color": "#eac77d",
        "path": "M25 74Q17 44 74 23Q77 63 25 74ZM25 74L63 37M38 62L36 47M48 53L48 37M48 53L65 51"
    },
    "branch": {
        "color": "#88c6ed",
        "path": "M50 76V27M50 52L28 39V27M50 62L73 45V27M43 27H57M21 27H35M66 27H80"
    },
    "tree": {
        "color": "#88c6ed",
        "path": "M46 77V61H28L37 48H26L39 35H32L50 18L68 35H61L74 48H63L72 61H54V77Z"
    },
    "network": {
        "color": "#88c6ed",
        "path": "M50 30L28 57L72 57ZM50 20A7 7 0 1 0 50 34A7 7 0 1 0 50 20ZM24 55A7 7 0 1 0 24 69A7 7 0 1 0 24 55ZM76 55A7 7 0 1 0 76 69A7 7 0 1 0 76 55M31 68L42 79M69 68L58 79"
    },
    "dna": {
        "color": "#88c6ed",
        "path": "M34 22Q74 40 50 51Q25 62 66 80M66 22Q26 40 50 51Q75 62 34 80M40 29H60M38 39H62M39 62H61M41 72H59"
    },
    "cycle": {
        "color": "#88c6ed",
        "path": "M27 39A25 25 0 0 1 73 39L74 26M73 39H60M73 62A25 25 0 0 1 27 62L26 75M27 62H40M44 50L49 55L59 44"
    },
    "page": {
        "color": "#64d8bf",
        "path": "M31 23H60L73 37V77H31ZM60 23V38H73M40 49H63M40 59H63M40 69H54"
    },
    "binoculars": {
        "color": "#64d8bf",
        "path": "M26 34H40L44 64H15ZM60 34H74L85 64H56ZM44 46H56M44 54H56M22 65A9 9 0 1 0 40 65M60 65A9 9 0 1 0 78 65"
    },
    "notebook": {
        "color": "#64d8bf",
        "path": "M30 22H74V78H30ZM23 33H36M23 46H36M23 60H36M23 72H36M47 59Q40 38 64 35Q70 56 47 59ZM47 59L60 43"
    },
    "globe": {
        "color": "#64d8bf",
        "path": "M50 23A27 27 0 1 0 50 77A27 27 0 1 0 50 23ZM50 23Q25 50 50 77Q75 50 50 23M23 50H77M29 36H71M29 64H71"
    },
    "butterfly": {
        "color": "#c6a0ed",
        "path": "M48 47Q12 11 22 49Q12 80 46 59M52 47Q88 11 78 49Q88 80 54 59M50 34V75M50 34L40 24M50 34L60 24"
    },
    "trail": {
        "color": "#c6a0ed",
        "path": "M27 76Q78 74 50 57Q19 41 68 31M61 23L73 30L64 40M29 64H34M42 39H47"
    },
    "mountain": {
        "color": "#c6a0ed",
        "path": "M17 75L43 26L60 53L68 40L86 75ZM34 43L44 47L51 40M61 54L69 59L74 53M43 26V16L56 19L43 23"
    },
    "camp": {
        "color": "#c6a0ed",
        "path": "M19 73L50 28L81 73ZM37 73L50 51L63 73M50 28V20M24 79H77M75 21V33M69 27H81"
    },
    "telescope": {
        "color": "#c6a0ed",
        "path": "M27 38L67 21L75 39L35 56ZM38 57L26 76M49 52V77M56 48L72 76M22 42L28 55M72 21L80 39"
    },
    "crown": {
        "color": "#f3b17a",
        "path": "M26 65L20 33L38 45L50 24L63 45L81 33L74 65ZM27 74H73M38 56H62"
    },
    "shield": {
        "color": "#f3b17a",
        "path": "M50 21L75 31V51Q74 70 50 82Q26 70 25 51V31ZM37 49L47 59L65 39"
    },
    "star": {
        "color": "#f3b17a",
        "path": "M50 19L59 39L81 42L65 58L69 80L50 69L31 80L35 58L19 42L41 39ZM50 35L54 46L66 48L57 56L59 67L50 62L41 67L43 56L34 48L46 46Z"
    }
}

#define PAGE_F1MAP      0x80
#define PAGE_F1DRIVER   0x40
#define PAGE_F1CARD     0x20
#define PAGE_F1HUD      (PAGE_F1TOWER|PAGE_F1CARD|PAGE_F1DRIVER|PAGE_F1MAP)
#define PAGE_F1TOWER    0x10
#define PAGE_NONE       0x00
#define PAGE_LOG        0x01
#define PAGE_MAP        0x02
#define PAGE_ATTHELINE  0x04
#define PAGE_CARINFO    0x08

#define RATL_START   0
#define RATL_BEHIND  0
#define RATL_LAPTIME 1
#define RATL_END     1

#define QATL_START   0
#define QATL_SECTORS 0
#define QATL_SECDIFF 1
#define QATL_IDEAL   2
#define QATL_END     2

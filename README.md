# Linux storage stack and raid
###### tags: `raid` `lvm` `pv` `storage stack`

> In the article, we'll focus on raid
> [name=ztex]
> 

## :memo: What's RAID?
RAID ("Redundant Array of Inexpensive Disks" or "Redundant Array of Independent Disks") is a data storage virtualization technology that combines multiple physical disk drive components into one or more logical units for the purposes of data redundancy, performance improvement, or both.

## :memo: LVM Concepts and Terminology
* see: https://www.digitalocean.com/community/tutorials/an-introduction-to-lvm-concepts-terminology-and-operations

LVM, or Logical Volume Management, is a storage device management technology that gives users the power to pool and abstract the physical layout of component storage devices for easier and flexible administration. Utilizing the device mapper Linux kernel framework, the current iteration, LVM2, can be used to gather existing storage devices into groups and allocate logical units from the combined space as needed.

> LVM, Logical Volume Management 的簡稱, 一種將 physical layout 抽象化的技術. 讓我們更好管理 storage deivces.
> 利用 linux kernel 的 device mapper framework (see: https://elixir.bootlin.com/linux/latest/source/include/linux/device-mapper.h) 可以將 storage devices 丟進 group 並且根據需求 allocate logical units.
> [name=ztex]
> 

* **Physical Volumes**:
LVM utility prefix: `pv...`
Description: Physical block devices or other disk-like devices (for example, other devices created by device mapper, like RAID arrays) are used by LVM as the raw building material for higher levels of abstraction. **Physical volumes are regular storage devices**. **LVM writes a header to the device to allocate it for management.**
> `pv` 是 physical volumes.
> physical volumes 對應 disk 上的 partition 或 disk 之類
> LVM 會把 header 寫進 device 來 allocate
> [name=ztex]
> 

* **Volume Groups**:
LVM utility prefix: `vg...`
Description: **LVM combines physical volumes into storage pools known as volume groups**. Volume groups abstract the characteristics of the underlying devices and function as a unified logical device with combined storage capacity of the component physical volumes.

> 一堆 physical volumes 組成 volume groups (就是一個 storage pool).
> volume groups 抽象化下層的 device. 同時方便上層分配 LV
> [name=ztex]
> 

* **Logical Volumes**:
LVM utility prefix: `lv...` (generic LVM utilities might begin with `lvm...`)
Description: **A volume group can be sliced up into any number of logical volumes**. **Logical volumes are functionally equivalent to partitions on a physical disk**, but with much more flexibility. Logical volumes are the **primary component that users and applications will interact with**.

> 有了 storage pool (volume group) 接下來切一切就變成了, 一塊塊的 **logical volumes**
> 可以看成 disk 的 partitions. (概念上啦)
> application 主要跟 lv 打交道
> [name=ztex]
> 

* **Extents**
Each volume within a volume group is segmented into **small, fixed-size chunks called extents**. The size of the extents is determined by the volume group (**all volumes within the group conform to the same extent size**).
The extents on a physical volume are called **physical extents**, while the extents of a **logical volume** are called logical extents. A logical volume is simply a **mapping that LVM maintains between logical and physical extents**. Because of this relationship, the extent size represents the smallest amount of space that can be allocated by LVM.
Extents are behind much of the flexibility and power of LVM. The logical extents that are presented as a unified device by LVM **do not have to map to continuous physical extents**. LVM can copy and reorganize the physical extents that compose a logical volume without any interruption to users. Logical volumes can also be easily expanded or shrunk by simply adding extents to or removing extents from the volume.

> group 裡面的 volumes 會切成 extents.
> physical volume 切出 physical extents. logical volume 切出 logical extents.
> 一個 volume group 里的 extent size 都一樣.
> LVM 有一組 logical extents 對照 physical extents 的 mapping. 但不需要是 map 連續的 physical extents.
> [name=ztex]
> 

![](https://i.imgur.com/pl5dh19.png)
```bash=
$> lsblk
...
sata4                             8:48   1 465.8G  0 disk
├─sata4p1                         8:49   1   2.4G  0 part
│ └─md0                           9:0    0   2.4G  0 raid1 /
├─sata4p2                         8:50   1     2G  0 part
│ └─md1                           9:1    0     2G  0 raid1 [SWAP]
└─sata4p5                         8:53   1 461.2G  0 part
  └─md3                           9:3    0 922.3G  0 raid5
    ├─vg1-syno_vg_reserved_area 251:2    0    12M  0 lvm
    ├─vg1-volume_2              251:3    0    10G  0 lvm
    │ └─cachedev_1              251:4    0    10G  0 dm    /volume2
    ├─vg1-volume_3              251:5    0    10G  0 lvm
    │ └─cachedev_2              251:6    0    10G  0 dm    /volume3
    └─vg1-volume_4              251:7    0    10G  0 lvm
      └─cachedev_3              251:8    0    10G  0 dm    /volume4
```


## :memo: What kinds of raid does Synology Inc. provide?

- [ ] storage stack(left); synology nas storage stack (920+) 
![](https://i.imgur.com/PmKH8QW.png)

* SHR
    * see: https://www.synology.com/zh-tw/knowledgebase/DSM/tutorial/Storage/What_is_Synology_Hybrid_RAID_SHR
    ![](https://i.imgur.com/jdxxuoh.png)

* Basic
* JBOD
![](https://i.imgur.com/9TS657j.png)
* RAID 1
![](https://i.imgur.com/UZusjRy.png)
* RAID 0
![](https://i.imgur.com/UQcnCDb.png)
* RAID 5
![](https://i.imgur.com/tTQy7Qa.png)
* RAID 6
![](https://i.imgur.com/5Wl516c.png)
* RAID 10
![](https://i.imgur.com/Wk54uQH.png)
* RAID F1
![](https://i.imgur.com/nC8hnHx.png)

> 比較特別的大概就是 SHR, RAID F1
> 這兩個是 Synology 特有的 RAID type
> SHR (Synology Hybrid RAID) 顧名思義就是混合型的 RAID
> 拿 RAID 5 來說, 對 RAID 熟悉的人大概就知道, Array size = Min Disk Size * (N-1)
> 好, 那今天假設你的 array 裡面有顆她媽小的硬碟你要怎麼辦? 你就是會一堆空間浪費
> 所以 Synology 就說, 浪費的硬碟空間也拿來做 RAID 好了, 最差的情況就是剩下兩顆硬碟, 為了保證都具有容錯能力, 所以兩個硬碟會走 RAID 1
> RAID F1, 顧名思義就是 Flash (base on RAID 5)
> 了解, SSD 的物理特性大概都知道 SSD 有所謂的 life span, 這跟他最多可擦除的次數有關, 可以看看 PE (program erase), 所以今天假設你的 RAID 都是 SSD 然後 PE 相近, 乾, 那如果有一天死, 不就大家一起死, 這樣 RAID 搞毛, 至少同時間不能大家一起走吧, 所以, RAID F1, 選醫顆倒楣鬼硬碟, 在她身上多寫一份 parity, 阿只要要寫入, 就一定會寫到 parity (see rcw vs. rmw), 所以理論上這顆硬碟應該比較容易先走人.
> [name=ztex]
> 

## :memo: resync: What is it? categories & operation
Also known as: rebuild、resync、data scrubbing、raid scrubbing
1. resync: kernel trigger, found imparity, update parity with data
2. request-resync: a.k.a raid data scrubbing, user trigger
3. recovery: After degrade, add new disk, data+parity => new data
4. reshape: if layout changed, e.g. raid1 -> raid5, 3 disk raid5 -> 4 disk raid5), update parity
5. check: did not repair

### resync in kernel
see: linux-4.4.x/Documentation/device-mapper/dm-raid.txt
see: linux-4.4.x/drivers/md/dm-raid.c
- [ ] /sys/block/mdX/md/sync_action
* idle: No synchronization action is being performed
* frozen: The current action has been halted.
* resync: initial synchronization or is resynchronizing after an unclean shutdown
* recover
* check
* repair
* reshape

```shell=
$> cat /proc/mdstat
$> echo xxx > /sys/block/mdX/md/sync_action
```
![](https://i.imgur.com/3TcUU36.png)

## :computer: RAID 5

### rmw vs rcw

#### rmw: read, modify, write
Original:
P = A ^ B ^ C
Write A: A’
means we got to write new parity P’ as well
P’ = A’ ^ B ^ C
because B ^ C = P ^ A
P’ = A’ ^ P ^ A
rmw: read original P and A, then get P’ with A’ ^ P ^ A

#### rcw: read, reconstruct, write
Original:
P = A ^ B ^ C
Write A: A’
P’ = A’ ^ B ^ C
rcw: read original B and C, then get P’ with A’ ^ B ^ C

#### Decision: rcw vs rmw ?
* rcw <= rmw, controller choose rcw, choose whose read cost is less
* if rcw == rmw, choose, because it does not depend on parity, less likely to go wrong
* echo 1 > /sys/block/mdX/md/enable_rmw
* When data is not consistent to parity, tend to make raid consistent, we prefer rcw over rmw
* see: `drivers/md/raid5.c`
    * handle_stripe_dirtying(..)
        * decide rcw or rmw

### Random write 4K, RAID 5
see: https://elixir.bootlin.com/linux/latest/source/drivers/md/raid5.h#475
stripe size = page size = 4KB (x86)
![](https://i.imgur.com/GJEWzvZ.png)

### Random write 4K, RAID 6
![](https://i.imgur.com/1PxJCQF.png)

## :memo: mdadm

* `cat /proc/mdstat`
    * ![](https://i.imgur.com/zdaFYWW.png)

* Create a new array
    * mdadm -C /dev/md3 --assume-clean -R -f -amd -n3 -z1048576 -l5  /dev/sda3 /dev/sdb3 /dev/sdc3
        * –assume-clean : not do resync
        * -n : device number
        * -z : size per component(KB)
        * -l : raid level
* Stop md
    * mdadm -S /dev/md3
* Assemble md
    * mdadm -A --scan
    * mdadm -A /dev/md3 /dev/sd[abc]3
* Remove component in md
    * mdadm /dev/md3 -f /dev/sda3; mdadm /dev/md3 -r /dev/sda3
* Add component to md
    * mdadm /dev/md3 -a /dev/sda3
* Show detail
    * mdadm --detail /dev/mdx
    * mdadm -D /dev/mdx
* Examine
    * mdadm -E /dev/sata1p5

## synomdmapper: mapping between `md` and `drive`
* github: https://github.com/tony2037/bit-newbie

* `/sys/block/mdX/`:
    * `slaves`: all the disks in the array
    * `/sys/block/md3/md/array_size`: literally
    * `/sys/block/md3/md/layout`: see https://elixir.bootlin.com/linux/latest/source/drivers/md/raid5.h#695
    * `/sys/block/md3/md/level`: RAID level
    * ` /sys/block/mdX/md/dev-X/`:
        * offset
        * size
        * slot: No in the array
    * `/sys/block/mdX/md/bitmap/`
        * write-intent log
        * 想像一下如果寫到一半斷電, 總不能全部重寫吧
        * 1 bit 代表多少可以自己設, 預設 4K (?)
        * write -> set 1 -> done -> set 0
    * see: https://www.kernel.org/doc/html/v4.15/admin-guide/md.html
    
* Features:
    * Given: md name, return: disk name, disk lba
    * Given: md name, disk name, return: md lba

* RAID1
    * Mapping (raidSector)
        * disk = 0
        * diskSector = raidSector
    * Reverse Mapping (disk, diskSector)
        * raidSector = diskSector

* Linear (JBOD)
    * Mapping (raidSector)
        * disk = raidSector / sectorsInDisk
        * diskSector = raidSector % sectorsInDisk
    * Reverse Mapping (disk, diskSector)
        * raidSector = disk * sectorsInDisk + diskSector

* RAID5
    * Mapping (raidSector)
        * chunk <- raidSector
        * disk = chunk % this->Disks.size();
        * diskSector = raidSector % (sectorsInChunk) + sector / sectorsInStrip * sectorsInChunk;
        * eg: mapping(5)
        * disk = 5 % 3 = 2; diskChunk = 2 = 5 / 2; note: 2 chunk a strip
    * Reverse Mapping (disk, diskSector)
        * group <- (diskSector, nDisks)
        * chunkOnMd = GROUP + DISKCHUNK + OFFSET (see diagonal line)
        * sectorOnMd = chunkOnMd * sectorsInChunk + offsetInChunk
    * ![](https://i.imgur.com/8WGzK9G.png)

* RAID6
    * ![](https://i.imgur.com/Z6lhI0g.png)

## Reference

* see:
    * [A Stripe-Oriented Write Performance Optimization for
RAID-Structured Storage Systems](http://www.nas-conference.org/NAS-2016/camera_ready/04.pdf)
    * [RAID4S-modthresh: Modifying the Write Selection
Algorithm to Classify Medium-Writes as Small-Writes](https://www.soe.ucsc.edu/sites/default/files/technical-reports/UCSC-SOE-12-10.pdf)
    * [lbatofile](https://github.com/sdgathman/lbatofile)
    * [Software RAID Manager](https://github.com/amanchadha/software-RAID)
    * `/linux-4.4.x/-/blob/master/Documentation/device-mapper/dm-raid.txt`
    * [md road-map: 2011](https://lwn.net/Articles/428206/)
    * [md linux manual](https://man7.org/linux/man-pages/man4/md.4.html)
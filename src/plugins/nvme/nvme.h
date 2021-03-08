#include <glib.h>
#include <glib-object.h>
#include <blockdev/utils.h>

#ifndef BD_NVME
#define BD_NVME

GQuark bd_nvme_error_quark (void);
#define BD_NVME_ERROR bd_nvme_error_quark ()

typedef enum {
    BD_NVME_ERROR_TECH_UNAVAIL,
    BD_NVME_ERROR_FAILED,
    BD_NVME_ERROR_DRIVE_GENERIC,
    BD_NVME_ERROR_DRIVE_COMMAND_SPECIFIC,
    BD_NVME_ERROR_DRIVE_IO,
    BD_NVME_ERROR_IO_FABRICS,
    BD_NVME_ERROR_IO_ZNS,
    BD_NVME_ERROR_IO_PATH,
} BDNVMEError;

typedef enum {
    BD_NVME_TECH_NVME = 0,
    BD_NVME_TECH_NVMEOF,
} BDNVMETech;

typedef enum {
    BD_NVME_TECH_MODE_INFO         = 1 << 0,
    BD_NVME_TECH_MODE_NAMESPACE    = 1 << 1,
    BD_NVME_TECH_MODE_INITIATOR    = 1 << 2,
} BDNVMETechMode;

/**
 * BDNVMEControllerInfo:
 * @feat_multiport: %TRUE if the NVM subsystem may contain more than one NVM subsystem port.
 * @feat_multictrl: %TRUE if the NVM subsystem may contain two or more controllers.
 *                  An NVM subsystem that contains multiple controllers may be used by multiple
 *                  hosts, or may provide multiple paths for a single host.
 * @feat_sriov: %TRUE if the controller is associated with an SR-IOV Virtual Function,
 *              %FALSE if the controller is associated with a PCI Function or a Fabrics connection.
 * @feat_ana_supported: %TRUE if the NVM subsystem supports Asymmetric Namespace Access (ANA) Reporting.
 * @ctrl_id: Controller ID, the NVM subsystem unique controller identifier associated with the controller.
 * @fguid: FRU GUID, a 128-bit value that is globally unique for a given Field Replaceable Unit.
 * @feat_format: %TRUE if the controller supports the Format NVM command.
 * @feat_format_all_ns: %TRUE if all namespaces in an NVM subsystem shall be configured with the same
 *                      attributes and a format (excluding secure erase) of any namespace results in
 *                      a format of all namespaces in an NVM subsystem, %FALSE if the controller supports
 *                      format on a per namespace basis.
 * @feat_ns_mgmt: %TRUE if the controller supports the Namespace Management and Attachment capability.
 * @feat_selftest: %TRUE if the controller supports the Device Self-test command.
 * @feat_one_selftest: %TRUE if the NVM subsystem supports only one device self-test operation in progress at a time.
 * @selftest_ext_time: Extended Device Self-test Time, if @feat_selftest is supported then this field
 *                     indicates the nominal amount of time in one minute units that the controller takes
 *                     to complete an extended device self-test operation when in power state 0.
 * @hmb_pref_size: Host Memory Buffer Preferred Size indicates the preferred size that the host
 *                 is requested to allocate for the Host Memory Buffer feature in bytes.
 * @hmb_min_size: Host Memory Buffer Minimum Size indicates the minimum size that the host
 *                is requested to allocate for the Host Memory Buffer feature in bytes.
 * @size_total: Total NVM Capacity in the NVM subsystem in bytes.
 * @size_unalloc: Unallocated NVM Capacity in the NVM subsystem in bytes.
 * @feat_sanitize_crypto: %TRUE if the controller supports the Crypto Erase sanitize operation.
 * @feat_sanitize_block: %TRUE if the controller supports the Block Erase sanitize operation.
 * @feat_sanitize_overw: %TRUE if the controller supports the Overwrite sanitize operation.
 * @feat_secure_erase_all: %TRUE in case any secure erase performed as part of a format operation
 *                         results in a secure erase of all namespaces in the NVM subsystem,
 *                         %FALSE in case any secure erase performed as part of a format results
 *                         in a secure erase of the particular namespace specified.
 * @feat_secure_erase_crypto: %TRUE if cryptographic erase is supported.
 * @num_namespaces: Maximum Number of Allowed Namespaces supported by the NVM subsystem.
 * @subsysnqn: NVM Subsystem NVMe Qualified Name, UTF-8 null terminated string.
 */
typedef struct BDNVMEControllerInfo {
    gboolean feat_multiport;
    gboolean feat_multictrl;
    gboolean feat_sriov;
    gboolean feat_ana_supported;
    guint16 ctrl_id;
    gchar *fguid;
    gboolean feat_format;
    gboolean feat_format_all_ns;
    gboolean feat_ns_mgmt;
    gboolean feat_selftest;
    gboolean feat_one_selftest;
    gint selftest_ext_time;
    guint64 hmb_pref_size;
    guint64 hmb_min_size;
    guint64 size_total;
    guint64 size_unalloc;
    gboolean feat_sanitize_crypto;
    gboolean feat_sanitize_block;
    gboolean feat_sanitize_overw;
    gboolean feat_secure_erase_all;
    gboolean feat_secure_erase_crypto;
    gint num_namespaces;
    gchar *subsysnqn;
} BDNVMEControllerInfo;

/**
 * BDNVMELBAFormatRelativePerformance:
 * Performance index of the LBA format relative to other LBA formats supported by the controller.
 * @BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_UNKNOWN: Unknown relative performance index.
 * @BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_BEST: Best performance.
 * @BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_BETTER: Better performance.
 * @BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_GOOD: Good performance.
 * @BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_DEGRADED: Degraded performance.
 */
typedef enum {
    BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_UNKNOWN = 0,
    BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_BEST = 1,
    BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_BETTER = 2,
    BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_GOOD = 3,
    BD_NVME_LBA_FORMAT_RELATIVE_PERFORMANCE_DEGRADED = 4
} BDNVMELBAFormatRelativePerformance;

/**
 * BDNVMELBAFormat:
 * Namespace LBA Format Data Structure.
 * @data_size: LBA data size (i.e. a sector size) in bytes.
 * @relative_performance: Relative Performance index, see #BDNVMELBAFormatRelativePerformance.
 */
typedef struct BDNVMELBAFormat {
    guint16 data_size;
    BDNVMELBAFormatRelativePerformance relative_performance;
} BDNVMELBAFormat;

/**
 * BDNVMENamespaceInfo:
 * @nsid: The Namespace Identifier (NSID).
 * @eui64: IEEE Extended Unique Identifier: a 64-bit IEEE Extended Unique Identifier (EUI-64)
 *         that is globally unique and assigned to the namespace when the namespace is created.
 *         Remains fixed throughout the life of the namespace and is preserved across namespace
 *         and controller operations.
 * @nguid: Namespace Globally Unique Identifier: a 128-bit value that is globally unique and
 *         assigned to the namespace when the namespace is created. Remains fixed throughout
 *         the life of the namespace and is preserved across namespace and controller operations.
 * @uuid: Namespace 128-bit Universally Unique Identifier (UUID) as specified in RFC 4122.
 * @nsize: Namespace Size: total size of the namespace in logical blocks. The number of logical blocks
 *         is based on the formatted LBA size (see @current_lba_format).
 * @ncap: Namespace Capacity: maximum number of logical blocks that may be allocated in the namespace
 *        at any point in time. The number of logical blocks is based on the formatted LBA size (see @current_lba_format).
 * @nuse: Namespace Utilization: current number of logical blocks allocated in the namespace.
 *        This field is smaller than or equal to the Namespace Capacity. The number of logical
 *        blocks is based on the formatted LBA size (see @current_lba_format).
 * @feat_thin: %TRUE if the namespace supports thin provisioning. Specifically, the Namespace Capacity
 *             reported may be less than the Namespace Size.
 * @feat_multipath_shared: %TRUE for the capability to attach the namespace to two or more controllers
 *                         in the NVM subsystem concurrently.
 * @feat_format_progress: %TRUE for the capability to report the percentage of the namespace
 *                        that remains to be formatted.
 * @format_progress_remaining: The percentage value remaining of a format operation in progress.
 * @write_protected: %TRUE if the namespace is currently write protected and all write access to the namespace shall fail.
 * @lba_formats: (array zero-terminated=1) (element-type BDNVMELBAFormat): A list of supported LBA Formats.
 * @current_lba_format: A LBA Format currently used for the namespace. Contains zeroes in case of
 *                      an invalid or no supported LBA Format reported.
 */
typedef struct BDNVMENamespaceInfo {
    guint32 nsid;
    gchar *eui64;
    gchar *uuid;
    gchar *nguid;
    guint64 nsize;
    guint64 ncap;
    guint64 nuse;
    gboolean feat_thin;
    gboolean feat_multipath_shared;
    gboolean feat_format_progress;
    guint8 format_progress_remaining;
    gboolean write_protected;
    BDNVMELBAFormat **lba_formats;
    BDNVMELBAFormat current_lba_format;
} BDNVMENamespaceInfo;

/**
 * BDNVMESmartLog:
 * @warning_crit_spare: Critical Warning - the available spare capacity has fallen below the threshold.
 * @warning_crit_temp: Critical Warning - a temperature is either greater than or equal to an over
 *                     temperature threshold; or less than or equal to an under temperature threshold.
 * @warning_crit_degraded: Critical Warning - the NVM subsystem reliability has been degraded due to
 *                         significant media related errors or any internal error that degrades
 *                         NVM subsystem reliability.
 * @warning_crit_ro: Critical Warning - all of the media has been placed in read only mode.
 *                   Unrelated to the write protection state of a namespace.
 * @warning_crit_volatile_mem: Critical Warning - the volatile memory backup device has failed.
 *                             Valid only if the controller has a volatile memory backup solution.
 * @warning_crit_pmr_ro: Critical Warning - Persistent Memory Region has become read-only or unreliable.
 * @avail_spare: Available Spare: a normalized percentage (0% to 100%) of the remaining spare capacity available.
 * @spare_thresh: Available Spare Threshold: a normalized percentage (0% to 100%) of the available spare threshold.
 * @percent_used: Percentage Used: a vendor specific estimate of the percentage drive life used based on the
 *                actual usage and the manufacturer's prediction. A value of %100 indicates that the estimated
 *                endurance has been consumed, but may not indicate an NVM subsystem failure.
 *                The value is allowed to exceed %100.
 * @total_data_read: An estimated calculation of total data read in bytes based on calculation of data
 *                   units read from the host. A value of %0 indicates that the number of Data Units Read
 *                   is not reported.
 * @total_data_written: An estimated calculation of total data written in bytes based on calculation
 *                      of data units written by the host. A value of %0 indicates that the number
 *                      of Data Units Written is not reported.
 * @ctrl_busy_time: Amount of time the controller is busy with I/O commands, reported in minutes.
 * @power_cycles: The number of power cycles.
 * @power_on_hours: The number of power-on hours, excluding a non-operational power state.
 * @unsafe_shutdowns: The number of unsafe shutdowns as a result of a Shutdown Notification not received prior to loss of power.
 * @media_errors: Media and Data Integrity Errors: the number of occurrences where the controller detected
 *                an unrecovered data integrity error (e.g. uncorrectable ECC, CRC checksum failure, or LBA tag mismatch).
 * @num_err_log_entries: Number of Error Information Log Entries: the number of Error Information log
 *                       entries over the life of the controller.
 * @temperature: Composite Temperature: temperature in degrees Celsius that represents the current composite
 *               temperature of the controller and associated namespaces or %-273 when not applicable.
 * @temp_sensors: Temperature Sensor 1-8: array of the current temperature reported by temperature sensors
 *                1-8 in degrees Celsius or %-273 when the particular sensor is not available.
 * @wctemp: Warning Composite Temperature Threshold (WCTEMP): indicates the minimum Composite Temperature (@temperature)
 *          value that indicates an overheating condition during which controller operation continues.
 *          A value of %0 indicates that no warning temperature threshold value is reported by the controller.
 * @cctemp: Critical Composite Temperature Threshold (CCTEMP): indicates the minimum Composite Temperature (@temperature)
 *          value that indicates a critical overheating condition (e.g., may prevent continued normal operation,
 *          possibility of data loss, automatic device shutdown, extreme performance throttling, or permanent damage).
 *          A value of %0 indicates that no critical temperature threshold value is reported by the controller.
 * @warning_temp_time: Warning Composite Temperature Time: the amount of time in minutes that the Composite Temperature (@temperature)
 *                     is greater than or equal to the Warning Composite Temperature Threshold (@wctemp) and less than the
 *                     Critical Composite Temperature Threshold (@cctemp).
 * @critical_temp_time: Critical Composite Temperature Time: the amount of time in minutes that the Composite Temperature (@temperature)
 *                      is greater than or equal to the Critical Composite Temperature Threshold (@cctemp).
 */
typedef struct BDNVMESmartLog {
    gboolean warning_crit_spare;
    gboolean warning_crit_temp;
    gboolean warning_crit_degraded;
    gboolean warning_crit_ro;
    gboolean warning_crit_volatile_mem;
    gboolean warning_crit_pmr_ro;
    guint8 avail_spare;
    guint8 spare_thresh;
    guint8 percent_used;
    guint64 total_data_read;
    guint64 total_data_written;
    guint64 ctrl_busy_time;
    guint64 power_cycles;
    guint64 power_on_hours;
    guint64 unsafe_shutdowns;
    guint64 media_errors;
    guint64 num_err_log_entries;
    gint temperature;
    gint temp_sensors[8];
    guint wctemp;
    guint cctemp;
    guint warning_temp_time;
    guint critical_temp_time;
} BDNVMESmartLog;

/**
 * BDNVMETransportType:
 * Transport Type.
 * @BD_NVME_TRANSPORT_TYPE_UNSPECIFIED: Not indicated
 * @BD_NVME_TRANSPORT_TYPE_RDMA: RDMA Transport
 * @BD_NVME_TRANSPORT_TYPE_FC: Fibre Channel Transport
 * @BD_NVME_TRANSPORT_TYPE_TCP: TCP Transport
 * @BD_NVME_TRANSPORT_TYPE_LOOP: Intra-host Transport (loopback)
 */
typedef enum {
    BD_NVME_TRANSPORT_TYPE_UNSPECIFIED = 0,
    BD_NVME_TRANSPORT_TYPE_RDMA        = 1,
    BD_NVME_TRANSPORT_TYPE_FC          = 2,
    BD_NVME_TRANSPORT_TYPE_TCP         = 3,
    BD_NVME_TRANSPORT_TYPE_LOOP        = 254
} BDNVMETransportType;

/**
 * BDNVMEErrorLogEntry:
 * @error_count: internal error counter, a unique identifier for the error.
 * @command_id: the Command Identifier of the command that the error is associated with or %0xffff if the error is not specific to a particular command.
 * @command_specific: Command Specific Information specific to @command_id.
 * @command_status: the Status code for the command that completed.
 * @command_error: translated command error in the BD_NVME_ERROR domain or %NULL in case @command_status indicates success.
 * @lba: the first LBA that experienced the error condition.
 * @nsid: the NSID of the namespace that the error is associated with.
 * @transport_type: type of the transport associated with the error.
 */
typedef struct BDNVMEErrorLogEntry {
    guint64 error_count;
    guint16 command_id;
    guint64 command_specific;
    guint16 command_status;
    GError *command_error;
    guint64 lba;
    guint32 nsid;
    BDNVMETransportType transport_type;
} BDNVMEErrorLogEntry;


void bd_nvme_controller_info_free (BDNVMEControllerInfo *info);
BDNVMEControllerInfo * bd_nvme_controller_info_copy (BDNVMEControllerInfo *info);

void bd_nvme_lba_format_free (BDNVMELBAFormat *fmt);
BDNVMELBAFormat * bd_nvme_lba_format_copy (BDNVMELBAFormat *fmt);

void bd_nvme_namespace_info_free (BDNVMENamespaceInfo *info);
BDNVMENamespaceInfo * bd_nvme_namespace_info_copy (BDNVMENamespaceInfo *info);

void bd_nvme_smart_log_free (BDNVMESmartLog *log);
BDNVMESmartLog * bd_nvme_smart_log_copy (BDNVMESmartLog *log);

void bd_nvme_error_log_entry_free (BDNVMEErrorLogEntry *entry);
BDNVMEErrorLogEntry * bd_nvme_error_log_entry_copy (BDNVMEErrorLogEntry *entry);


/*
 * If using the plugin as a standalone library, the following functions should
 * be called to:
 *
 * check_deps() - check plugin's dependencies, returning TRUE if satisfied
 * init()       - initialize the plugin, returning TRUE on success
 * close()      - clean after the plugin at the end or if no longer used
 *
 */
gboolean bd_nvme_check_deps (void);
gboolean bd_nvme_init (void);
void     bd_nvme_close (void);

gboolean bd_nvme_is_tech_avail (BDNVMETech tech, guint64 mode, GError **error);


BDNVMEControllerInfo * bd_nvme_get_controller_info   (const gchar *device, GError **error);
BDNVMENamespaceInfo *  bd_nvme_get_namespace_info    (const gchar *device, GError **error);
BDNVMESmartLog *       bd_nvme_get_smart_log         (const gchar *device, GError **error);
BDNVMEErrorLogEntry ** bd_nvme_get_error_log_entries (const gchar *device, GError **error);


#endif  /* BD_NVME */

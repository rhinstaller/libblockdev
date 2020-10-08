#include <glib.h>

#ifndef BD_FS_GENERIC
#define BD_FS_GENERIC

gboolean bd_fs_wipe (const gchar *device, gboolean all, GError **error);
gboolean bd_fs_clean (const gchar *device, GError **error);
gchar* bd_fs_get_fstype (const gchar *device,  GError **error);

gboolean bd_fs_freeze (const gchar *mountpoint, GError **error);
gboolean bd_fs_unfreeze (const gchar *mountpoint, GError **error);

typedef enum {
    BD_FS_MKFS_LABEL     = 1 << 0,
    BD_FS_MKFS_UUID      = 1 << 1,
    BD_FS_MKFS_DRY_RUN   = 1 << 2,
    BD_FS_MKFS_NODISCARD = 1 << 3,
} BDFsMkfsOptionsFlags;

typedef struct BDFsMkfsOptions {
    gchar *label;
    gchar *uuid;
    gboolean dry_run;
    gboolean no_discard;
} BDFsMkfsOptions;

gboolean bd_fs_mkfs (const gchar *device, const gchar *fsname, BDFsMkfsOptions *options, GError **error);

gboolean bd_fs_resize (const gchar *device, guint64 new_size, GError **error);
gboolean bd_fs_repair (const gchar *device, GError **error);
gboolean bd_fs_check (const gchar *device, GError **error);
gboolean bd_fs_set_label (const gchar *device, const gchar *label, GError **error);
gboolean bd_fs_set_uuid (const gchar *device, const gchar *uuid, GError **error);
guint64 bd_fs_get_size (const gchar *device, GError **error);
guint64 bd_fs_get_free_space (const gchar *device, GError **error);

typedef enum {
    BD_FS_OFFLINE_SHRINK = 1 << 1,
    BD_FS_OFFLINE_GROW = 1 << 2,
    BD_FS_ONLINE_SHRINK = 1 << 3,
    BD_FS_ONLINE_GROW = 1 << 4
} BDFsResizeFlags;

gboolean bd_fs_can_mkfs (const gchar *type, BDFsMkfsOptionsFlags *options, gchar **required_utility, GError **error);
gboolean bd_fs_can_resize (const gchar *type, BDFsResizeFlags *mode, gchar **required_utility, GError **error);
gboolean bd_fs_can_check (const gchar *type, gchar **required_utility, GError **error);
gboolean bd_fs_can_repair (const gchar *type, gchar **required_utility, GError **error);
gboolean bd_fs_can_set_label (const gchar *type, gchar **required_utility, GError **error);
gboolean bd_fs_can_set_uuid (const gchar *type, gchar **required_utility, GError **error);
gboolean bd_fs_can_get_size (const gchar *type, gchar **required_utility, GError **error);
gboolean bd_fs_can_get_free_space (const gchar *type, gchar **required_utility, GError **error);

#endif  /* BD_FS_GENERIC */

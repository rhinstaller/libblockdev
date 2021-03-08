/*
 * Copyright (C) 2014-2021 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Tomas Bzatek <tbzatek@redhat.com>
 */

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>

#include <libnvme.h>
#include <uuid/uuid.h>

#include <blockdev/utils.h>
#include <check_deps.h>
#include "nvme.h"
#include "nvme-private.h"


/**
 * bd_nvme_error_quark: (skip)
 */
GQuark bd_nvme_error_quark (void)
{
    return g_quark_from_static_string ("g-bd-nvme-error-quark");
}



static inline const gchar * _nvme_generic_status_to_string (guint status)
{
    /*
     * Generic Command Status Codes:
     */
    switch (status) {
    case NVME_SC_INVALID_OPCODE:       /* 0x1 */
        return "INVALID_OPCODE: The associated command opcode field is not valid";
    case NVME_SC_INVALID_FIELD:        /* 0x2 */
        return "INVALID_FIELD: A reserved coded value or an unsupported value in a defined field";
    case NVME_SC_CMDID_CONFLICT:       /* 0x3 */
        return "CMDID_CONFLICT: The command identifier is already in use";
    case NVME_SC_DATA_XFER_ERROR:      /* 0x4 */
        return "DATA_XFER_ERROR: Error while trying to transfer the data or metadata";
    case NVME_SC_POWER_LOSS:           /* 0x5 */
        return "POWER_LOSS: Command aborted due to power loss notification";
    case NVME_SC_INTERNAL:             /* 0x6 */
        return "INTERNAL: The command was not completed successfully due to an internal error";
    case NVME_SC_ABORT_REQ:            /* 0x7 */
        return "ABORT_REQ: The command was aborted due to a Command Abort request";
    case NVME_SC_ABORT_QUEUE:          /* 0x8 */
        return "ABORT_QUEUE: The command was aborted due to a Delete I/O Submission Queue request";
    case NVME_SC_FUSED_FAIL:           /* 0x9 */
        return "FUSED_FAIL: The command was aborted due to the other command in a fused operation failing";
    case NVME_SC_FUSED_MISSING:        /* 0xa */
        return "FUSED_MISSING: The command was aborted due to a Missing Fused Command";
    case NVME_SC_INVALID_NS:           /* 0xb */
        return "INVALID_NS: The namespace or the format of that namespace is invalid";
    case NVME_SC_CMD_SEQ_ERROR:        /* 0xc */
        return "CMD_SEQ_ERROR: The command was aborted due to a protocol violation in a multicommand sequence";
    case NVME_SC_SGL_INVALID_LAST:     /* 0xd */
        return "SGL_INVALID_LAST: The command includes an invalid SGL Last Segment or SGL Segment descriptor";
    case NVME_SC_SGL_INVALID_COUNT:    /* 0xe */
        return "SGL_INVALID_COUNT: There is an SGL Last Segment descriptor or an SGL Segment descriptor in a location other than the last descriptor of a segment based on the length indicated";
    case NVME_SC_SGL_INVALID_DATA:     /* 0xf */
        return "SGL_INVALID_DATA: This may occur if the length of a Data SGL is too short";
    case NVME_SC_SGL_INVALID_METADATA: /* 0x10 */
        return "SGL_INVALID_METADATA: This may occur if the length of a Metadata SGL is too short";
    case NVME_SC_SGL_INVALID_TYPE:     /* 0x11 */
        return "SGL_INVALID_TYPE: The type of an SGL Descriptor is a type that is not supported by the controller";
    case NVME_SC_CMB_INVALID_USE:      /* 0x12 */
        return "CMB_INVALID_USE: The attempted use of the Controller Memory Buffer is not supported by the controller";
    case NVME_SC_PRP_INVALID_OFFSET:   /* 0x13 */
        return "PRP_INVALID_OFFSET: The Offset field for a PRP entry is invalid";
    case NVME_SC_AWU_EXCEEDED:         /* 0x14 */
        return "AWU_EXCEEDED: The length specified exceeds the atomic write unit size";
    case NVME_SC_OP_DENIED:            /* 0x15 */
        return "OPERATION_DENIED: The command was denied due to lack of access rights";
    case NVME_SC_SGL_INVALID_OFFSET:   /* 0x16 */
        return "SGL_INVALID_OFFSET: The offset specified in a descriptor is invalid";
    /* missing 0x17 */
    case NVME_SC_HOSTID_FORMAT:        /* 0x18 */
        return "HOSTID_FORMAT: The NVM subsystem detected the simultaneous use of 64-bit and 128-bit Host Identifier values on different controllers";
    case NVME_SC_KAT_EXPIRED:          /* 0x19 */
        return "KAT_EXPIRED: The Keep Alive Timer expired";
    case NVME_SC_KAT_INVALID:          /* 0x1a */
        return "KAT_INVALID: The Keep Alive Timeout value specified is invalid";
    case NVME_SC_CMD_ABORTED_PREMEPT:  /* 0x1b */
        return "ABORTED_PREMEPT: The command was aborted due to a Reservation Acquire command with the Reservation Acquire Action (RACQA) set to 010b (Preempt and Abort)";
    case NVME_SC_SANITIZE_FAILED:      /* 0x1c */
        return "SANITIZE_FAILED: The most recent sanitize operation failed and no recovery actions has been successfully completed";
    case NVME_SC_SANITIZE_IN_PROGRESS: /* 0x1d */
        return "SANITIZE_IN_PROGRESS: The requested function is prohibited while a sanitize operation is in progress";
    case NVME_SC_SGL_INVALID_GRANULARITY:  /* 0x1e */
        return "SGL_INVALID_GRANULARITY: SGL Data Block Granularity Invalid: The Address alignment or Length granularity for an SGL Data Block descriptor is invalid";
    case NVME_SC_CMD_IN_CMBQ_NOT_SUPP: /* 0x1f */
        return "CMD_IN_CMBQ_NOT_SUPP: Command Not Supported for Queue in CMB: The implementation does not support submission of the command to a Submission Queue in the Controller Memory Buffer or command completion to a Completion Queue in the Controller Memory Buffer";
    case NVME_SC_NS_WRITE_PROTECTED:   /* 0x20 */
        return "NS_WRITE_PROTECTED: The command is prohibited while the namespace is write protected by the host.";
    case NVME_SC_CMD_INTERRUPTED:      /* 0x21 */
        return "CMD_INTERRUPTED: Command processing was interrupted and the controller is unable to successfully complete the command. The host should retry the command.";
    case NVME_SC_TRAN_TPORT_ERROR:     /* 0x22 */
        return "TRAN_TPORT_ERROR: A transient transport error was detected";
    case NVME_SC_LBA_RANGE:            /* 0x80 */
        return "LBA_RANGE: The command references a LBA that exceeds the size of the namespace";
    case NVME_SC_CAP_EXCEEDED:         /* 0x81 */
        return "CAP_EXCEEDED: The execution of the command has caused the capacity of the namespace to be exceeded";
    case NVME_SC_NS_NOT_READY:         /* 0x82 */
        return "NS_NOT_READY: The namespace is not ready to be accessed as a result of a condition other than a condition that is reported as an Asymmetric Namespace Access condition";
    case NVME_SC_RESERVATION_CONFLICT: /* 0x83 */
        return "RESERVATION_CONFLICT: The command was aborted due to a conflict with a reservation held on the accessed namespace";
    case NVME_SC_FORMAT_IN_PROGRESS:   /* 0x84 */
        return "FORMAT_IN_PROGRESS: A Format NVM command is in progress on the namespace.";
    default:
        return "Unknown status code";
    }
}

static inline const gchar * _nvme_cmd_specific_status_to_string (guint status)
{
    /*
     * Command Specific Status Codes:
     */
    switch (status) {
    case NVME_SC_CQ_INVALID:                    /* 0x00 */
        return "CQ_INVALID: The Completion Queue identifier specified in the command does not exist";
    case NVME_SC_QID_INVALID:                   /* 0x01 */
        return "QID_INVALID: The creation of the I/O Completion Queue failed due to an invalid queue identifier specified as part of the command. An invalid queue identifier is one that is currently in use or one that is outside the range supported by the controller";
    case NVME_SC_QUEUE_SIZE:                    /* 0x02 */
        return "QUEUE_SIZE: The host attempted to create an I/O Completion Queue with an invalid number of entries";
    case NVME_SC_ABORT_LIMIT:                   /* 0x03 */
        return "ABORT_LIMIT: The number of concurrently outstanding Abort commands has exceeded the limit indicated in the Identify Controller data structure";
    case NVME_SC_ABORT_MISSING:                 /* 0x04 */
        return "ABORT_MISSING: The abort command is missing";
    case NVME_SC_ASYNC_LIMIT:                   /* 0x05 */
        return "ASYNC_LIMIT: The number of concurrently outstanding Asynchronous Event Request commands has been exceeded";
    case NVME_SC_FIRMWARE_SLOT:                 /* 0x06 */
        return "FIRMWARE_SLOT: The firmware slot indicated is invalid or read only. This error is indicated if the firmware slot exceeds the number supported";
    case NVME_SC_FIRMWARE_IMAGE:                /* 0x07 */
        return "FIRMWARE_IMAGE: The firmware image specified for activation is invalid and not loaded by the controller";
    case NVME_SC_INVALID_VECTOR:                /* 0x08 */
        return "INVALID_VECTOR: The creation of the I/O Completion Queue failed due to an invalid interrupt vector specified as part of the command";
    case NVME_SC_INVALID_LOG_PAGE:              /* 0x09 */
        return "INVALID_LOG_PAGE: The log page indicated is invalid. This error condition is also returned if a reserved log page is requested";
    case NVME_SC_INVALID_FORMAT:                /* 0x0a */
        return "INVALID_FORMAT: The LBA Format specified is not supported. This may be due to various conditions";
    case NVME_SC_FW_NEEDS_CONV_RESET:           /* 0x0b */
        return "FW_NEEDS_CONVENTIONAL_RESET: The firmware commit was successful, however, activation of the firmware image requires a conventional reset";
    case NVME_SC_INVALID_QUEUE:                 /* 0x0c */
        return "INVALID_QUEUE: This error indicates that it is invalid to delete the I/O Completion Queue specified. The typical reason for this error condition is that there is an associated I/O Submission Queue that has not been deleted.";
    case NVME_SC_FEATURE_NOT_SAVEABLE:          /* 0x0d */
        return "FEATURE_NOT_SAVEABLE: The Feature Identifier specified does not support a saveable value";
    case NVME_SC_FEATURE_NOT_CHANGEABLE:        /* 0x0e */
        return "FEATURE_NOT_CHANGEABLE: The Feature Identifier is not able to be changed";
    case NVME_SC_FEATURE_NOT_PER_NS:            /* 0x0f */
        return "FEATURE_NOT_PER_NS: The Feature Identifier specified is not namespace specific. The Feature Identifier settings apply across all namespaces";
    case NVME_SC_FW_NEEDS_SUBSYS_RESET:         /* 0x10 */
        return "FW_NEEDS_SUBSYSTEM_RESET: The firmware commit was successful, however, activation of the firmware image requires an NVM Subsystem";
    case NVME_SC_FW_NEEDS_RESET:                /* 0x11 */
        return "FW_NEEDS_RESET: The firmware commit was successful; however, the image specified does not support being activated without a reset";
    case NVME_SC_FW_NEEDS_MAX_TIME:             /* 0x12 */
        return "FW_NEEDS_MAX_TIME_VIOLATION: The image specified if activated immediately would exceed the Maximum Time for Firmware Activation (MTFA) value reported in Identify Controller. To activate the firmware, the Firmware Commit command needs to be re-issued and the image activated using a reset";
    case NVME_SC_FW_ACTIVATE_PROHIBITED:        /* 0x13 */
        return "FW_ACTIVATION_PROHIBITED: The image specified is being prohibited from activation by the controller for vendor specific reasons";
    case NVME_SC_OVERLAPPING_RANGE:             /* 0x14 */
        return "OVERLAPPING_RANGE: This error is indicated if the firmware image has overlapping ranges";
    case NVME_SC_NS_INSUFFICIENT_CAP:           /* 0x15 */
        return "NS_INSUFFICIENT_CAPACITY: Creating the namespace requires more free space than is currently available. The Command Specific Information field of the Error Information Log specifies the total amount of NVM capacity required to create the namespace in bytes";
    case NVME_SC_NS_ID_UNAVAILABLE:             /* 0x16 */
        return "NS_ID_UNAVAILABLE: The number of namespaces supported has been exceeded";
    /* missing 0x17 */
    case NVME_SC_NS_ALREADY_ATTACHED:           /* 0x18 */
        return "NS_ALREADY_ATTACHED: The controller is already attached to the namespace specified";
    case NVME_SC_NS_IS_PRIVATE:                 /* 0x19 */
        return "NS_IS_PRIVATE: The namespace is private and is already attached to one controller";
    case NVME_SC_NS_NOT_ATTACHED:               /* 0x1a */
        return "NS_NOT_ATTACHED: The request to detach the controller could not be completed because the controller is not attached to the namespace";
    case NVME_SC_THIN_PROV_NOT_SUPP:            /* 0x1b */
        return "THIN_PROVISIONING_NOT_SUPPORTED: Thin provisioning is not supported by the controller";
    case NVME_SC_CTRL_LIST_INVALID:             /* 0x1c */
        return "CONTROLLER_LIST_INVALID: The controller list provided is invalid";
    case NVME_SC_SELF_TEST_IN_PROGRESS:         /* 0x1d */
        return "SELF_TEST_IN_PROGRESS: The controller or NVM subsystem already has a device self-test operation in process.";
    case NVME_SC_BP_WRITE_PROHIBITED:           /* 0x1e */
        return "BOOT PARTITION WRITE PROHIBITED: The command is trying to modify a Boot Partition while it is locked";
    case NVME_SC_INVALID_CTRL_ID:               /* 0x1f */
        return "INVALID_CTRL_ID: An invalid Controller Identifier was specified.";
    case NVME_SC_INVALID_SEC_CTRL_STATE:        /* 0x20 */
        return "INVALID_SECONDARY_CTRL_STATE: The action requested for the secondary controller is invalid based on the current state of the secondary controller and its primary controller";
    case NVME_SC_INVALID_CTRL_RESOURCES:        /* 0x21 */
        return "INVALID_NUM_CTRL_RESOURCE: The specified number of Flexible Resources is invalid";
    case NVME_SC_INVALID_RESOURCE_ID:           /* 0x22 */
        return "INVALID_RESOURCE_ID: At least one of the specified resource identifiers was invalid";
    case NVME_SC_PMR_SAN_PROHIBITED:            /* 0x23 */
        return "Sanitize Prohibited While Persistent Memory Region is Enabled: A sanitize operation is prohibited while the Persistent Memory Region is enabled";
    case NVME_SC_ANA_GROUP_ID_INVALID:          /* 0x24 */
        return "ANA_INVALID_GROUP_ID: The specified ANA Group Identifier (ANAGRPID) is not supported in the submitted command";
    case NVME_SC_ANA_ATTACH_FAILED:             /* 0x25 */
        return "ANA_ATTACH_FAILED: The controller is not attached to the namespace as a result of an ANA condition";
    /*
     * I/O Command Set Specific - NVM commands:
     */
    case NVME_SC_BAD_ATTRIBUTES:           /* 0x80 */
        return "BAD_ATTRIBUTES: Conflicting Dataset Management Attributes";
    case NVME_SC_INVALID_PI:               /* 0x81 */
        return "INVALID_PI: Invalid Protection Information";
    case NVME_SC_READ_ONLY:                /* 0x82 */
        return "READ_ONLY: Attempted Write to Read Only Range";
    default:
        return "Unknown command-specific status code";
    }
}

static inline const gchar * _nvme_fabrics_status_to_string (guint status)
{
    /*
     * I/O Command Set Specific - Fabrics commands:
     */
    switch (status) {
    case NVME_SC_CONNECT_FORMAT:           /* 0x80 */
        return "CONNECT_FORMAT: Incompatible Format: The NVM subsystem does not support the record format specified by the host";
    case NVME_SC_CONNECT_CTRL_BUSY:        /* 0x81 */
        return "CONNECT_CTRL_BUSY: Controller Busy: The controller is already associated with a host";
    case NVME_SC_CONNECT_INVALID_PARAM:    /* 0x82 */
        return "CONNECT_INVALID_PARAM: Connect Invalid Parameters: One or more of the command parameters";
    case NVME_SC_CONNECT_RESTART_DISC:     /* 0x83 */
        return "CONNECT_RESTART_DISC: Connect Restart Discovery: The NVM subsystem requested is not available";
    case NVME_SC_CONNECT_INVALID_HOST:     /* 0x84 */
        return "CONNECT_INVALID_HOST: Connect Invalid Host: The host is either not allowed to establish an association to any controller in the NVM subsystem or the host is not allowed to establish an association to the specified controller";
    case NVME_SC_DISCONNECT_INVALID_QTYPE: /* 0x85 */
        return "DISCONNECT_INVALID_QTYPE: Invalid Queue Type: The command was sent on the wrong queue type";
    case NVME_SC_DISCOVERY_RESTART:        /* 0x90 */
        return "DISCOVERY_RESTART: Discover Restart: The snapshot of the records is now invalid or out of date";
    case NVME_SC_AUTH_REQUIRED:            /* 0x91 */
        return "AUTH_REQUIRED: Authentication Required: NVMe in-band authentication is required and the queue has not yet been authenticated";
    default:
        return "Unknown NVMeoF status code";
    }
}

static inline const gchar * _nvme_zns_status_to_string (guint status)
{
    /*
     * I/O Command Set Specific - ZNS commands:
     */
    switch (status) {
    case NVME_SC_ZNS_BOUNDARY_ERROR:    /* 0xb8 */
        return "ZNS_BOUNDARY_ERROR: Invalid Zone Boundary crossing";
    case NVME_SC_ZNS_FULL:              /* 0xb9 */
        return "ZNS_FULL: The accessed zone is in ZSF:Full state";
    case NVME_SC_ZNS_READ_ONLY:         /* 0xba */
        return "ZNS_READ_ONLY: The accessed zone is in ZSRO:Read Only state";
    case NVME_SC_ZNS_OFFLINE:           /* 0xbb */
        return "ZNS_OFFLINE: The access zone is in ZSO:Offline state";
    case NVME_SC_ZNS_INVALID_WRITE:     /* 0xbc */
        return "ZNS_INVALID_WRITE: The write to zone was not at the write pointer offset";
    case NVME_SC_ZNS_TOO_MANY_ACTIVE:   /* 0xbd */
        return "ZNS_TOO_MANY_ACTIVE: The controller does not allow additional active zones";
    case NVME_SC_ZNS_TOO_MANY_OPENS:    /* 0xbe */
        return "ZNS_TOO_MANY_OPENS: The controller does not allow additional open zones";
    case NVME_SC_ZNS_INVAL_TRANSITION:  /* 0xbf */
        return "ZNS_INVAL_TRANSITION: The zone state change was invalid";
    }
    return NULL;
}

void _nvme_status_to_error (gint status, gboolean fabrics, GError **error)
{
    guint sc;
    const gchar *s;

    if (error == NULL)
        return;

    sc = nvme_status_code (status);
    switch (nvme_status_code_type (status)) {
        case NVME_SCT_GENERIC:
            g_set_error_literal (error, BD_NVME_ERROR, BD_NVME_ERROR_DRIVE_GENERIC,
                                 _nvme_generic_status_to_string (sc));
            break;
        case NVME_SCT_CMD_SPECIFIC:
            s = _nvme_zns_status_to_string (sc);
            if (s != NULL)
                g_set_error_literal (error, BD_NVME_ERROR, BD_NVME_ERROR_IO_ZNS, s);
            else
            if (fabrics)
                g_set_error_literal (error, BD_NVME_ERROR, BD_NVME_ERROR_IO_FABRICS,
                                     _nvme_fabrics_status_to_string (sc));
            else
                g_set_error_literal (error, BD_NVME_ERROR, BD_NVME_ERROR_DRIVE_COMMAND_SPECIFIC,
                                     _nvme_cmd_specific_status_to_string (sc));
            break;
        /* TODO: ANA, path-related */
        default:
            g_set_error (error, BD_NVME_ERROR, BD_NVME_ERROR_DRIVE_GENERIC,
                         "Unknown error code %x", status);
    }
}

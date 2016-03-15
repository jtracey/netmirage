#pragma once

// This module defines several functions that enable manipulation of virtual
// networks. The functions are implemented using direct interaction with the
// kernel, since they represent one of the most significant performance
// bottlenecks. This module is not thread safe. Namespace changes will affect
// the entire process. Some operations may be asynchronous or synchronous.
// Asynchronous calls will not report kernel errors.

// See the GOTCHAS file for common issues and misconceptions related to this
// module, or if the code stops working in a new kernel version.

#include <stdbool.h>
#include <stdint.h>

typedef struct netContext_s netContext;

// Initializes the network configuration module. Max length of namespacePrefix
// is theoretically PATH_MAX-1. Returns 0 on success or an error code otherwise.
int netInit(const char* namespacePrefix);

// Creates a new namespace with the given name. Visible to "ip". Automatically
// switches to the new namespace when called. Returns a context for the new
// namespace on success, or NULL on error. If err is not NULL, it is set to the
// error code on error. If an error occurs, the active namespace may no longer
// be valid.
netContext* netOpenNamespace(const char* name, int* err, bool excl);

// Frees resources associated with a context. This does not delete the
// underlying namespace, or cause the active namespace to switch.
void netCloseNamespace(netContext* ctx);

// Deletes a namespace. Returns 0 on success. Any associated contexts should be
// freed first. If the active namespace is deleted, the namespace will live
// until all bound processes are closed or switch to another namespace.
int netDeleteNamespace(const char* name);

// Switches the active namespace for the process. If the context is NULL,
// switches to the default network namespace for the PID namespace. Returns 0
// on success or an error code otherwise.
int netSwitchNamespace(netContext* ctx);

// Creates a virtual Ethernet pair of interfaces with endpoints in the given
// namespaces.
int netCreateVethPair(const char* name1, const char* name2, netContext* ctx1, netContext* ctx2, bool sync);

// Returns the interface index for an interface. On error, returns -1 and sets
// err (if provided) to the error code.
int netGetInterfaceIndex(netContext* ctx, const char* name, int* err);

// Adds an IPv4 address to an interface. addr should be an address in network
// byte order. subnetBits specifies the prefix length of the subnet. Returns 0
// on success or an error code otherwise.
int netAddInterfaceAddrIPv4(netContext* ctx, int devIdx, uint32_t addr, uint8_t subnetBits, uint32_t broadcastAddr, uint32_t anycastAddr, bool sync);

// Deletes an IPv4 address from the interface. Interfaces may have multiple
// addresses; this deletes only one of them. Returns 0 on success or an error
// code otherwise.
int netDelInterfaceAddrIPv4(netContext* ctx, int devIdx, bool sync);

// Brings an interface up or shuts it down. Returns 0 on success or an error
// code otherwise.
int netSetInterfaceUp(netContext* ctx, const char* name, bool up);

// Turns GRO (generic receive offload) on or off for an interface. Returns 0 on
// success or an error code otherwise.
int netSetInterfaceGro(netContext* ctx, const char* name, bool enabled);

// Uses Linux Traffic Control to apply shaping to outgoing packets on an
// interface. By default, interfaces have no delays or bandwidth limits.
// lossRate should be a value between 0.0 and 1.0. Passing 0.0 for rateMbit
// means that no limits are applied. queueLen specifies the maximum number of
// packets in the queue before packets are dropped. A value of 0 for queueLen
// uses the traffic control default value. Returns 0 on success or an error code
// otherwise.
int netSetEgressShaping(netContext* ctx, int devIdx, double delayMs, double jitterMs, double lossRate, double rateMbit, uint32_t queueLen, bool sync);

// Enables or disables packet routing between interfaces in the active
// namespace. Returns 0 on success or an error code otherwise.
int netSetForwarding(bool enabled);

// Adds a static routing entry to the main routing table. The destination is
// given by dstAddr with the subnetBits most significant bits specifying the
// subnet. Packets are routed via the specified gatewayAddr. dstDevIdx
// identifies the interface through which the packets should be sent. The
// target gateway must be reachable when this command is executed. Moreover, the
// all bits in dstAddr not covered by subnetBits should be set to 0. If
// gatewayAddr is 0, then no gateway is used. Returns 0 on success or an error
// code otherwise.
int netAddRoute(netContext* ctx, uint32_t dstAddr, uint8_t subnetBits, uint32_t gatewayAddr, int dstDevIdx, bool sync);

<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Cloud services (scaffold — no code yet)

Directory reserved for the optional SS-SP cloud services. **Status: empty
epic-gated scaffold.** First code lands with EPIC-21 stories; nothing here
builds or runs today.

| Directory               | Planned contents                                  |
|-------------------------|---------------------------------------------------|
| `relay/`                | Opt-in message relay                              |
| `provisioning-service/` | Fleet provisioning / enrollment                   |
| `fleet-console/`        | Fleet management console                          |
| `plugin-registry/`      | Signed plugin registry                            |

Constitutional constraint: devices must operate **indefinitely without any
cloud contact** (NF-REL-01); every service here is optional and opt-in
(`07_BUSINESS_MODEL_AND_OPEN_SOURCE.md`). Planning of record:
`docs/portfolio/epics/EPIC-21-cloud-services/`.

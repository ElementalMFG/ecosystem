# Open Assurance Statement

*Signed public commitments by the SS-SP founding vendor entity to the community and to future contributors and customers. Anchored in the transparency log at first release.*

## Community-edition licensing

The community-edition firmware and companion apps of SS-SP are and will remain licensed under Apache-2.0. The founding vendor entity irrevocably grants this license for all versions it publishes and commits **not** to retroactively relicense already-published code.

## Foundation transition

At the point where the SS-SP program reaches the Phase 2 governance criteria defined in `06_GOVERNANCE.md`, the founding vendor entity will transfer:

- The "SS-SP", "Seekie-Speakie", "Seekie", "Speakie", and "SS-SP-Certified" word marks;
- The shield/dot and radar-cursor design marks;
- All trademark registrations;
- Domain names of the project's official presence;
- Ownership of core project infrastructure (main repos, mailing lists, chat, CI);

to the neutral SS-SP Foundation (Linux Foundation, Apache, or equivalent recognized host).

Failing that transition, the founding vendor will publish the pre-agreed backup path for the trademarks and infrastructure to an independent open-source steward organisation.

## No paywall for essentials

The founding vendor commits that the following capabilities will remain available in the community-edition firmware, at no cost, on every device we ship:

- One-to-one text messaging.
- Group text messaging.
- Voice messaging / PTT (subject to hardware capability).
- Presence and location sharing.
- SOS and emergency beacons.
- Reticulum + LXMF connectivity.
- LoRa and Wi-Fi/BLE bearer support (subject to hardware).
- OTA updates (community releases).
- Interoperability with Meshtastic devices (wire compat).

Paid tiers may add fleet management, enterprise support, extended cloud services, premium plugins, and commercial certification. Paid tiers **may not** disable, degrade, or paywall the essentials above on shipped hardware.

## Security-fix commitment

The founding vendor commits to shipping security fixes for every device SKU sold, for a minimum of **five (5) years** from the last commercial shipment date of that SKU. LTS firmware branches are provided for this purpose.

## No user-data commercialization

The founding vendor commits:

- No advertising or third-party trackers in firmware or first-party companion apps.
- No sale of user data, message content, contacts, or location history.
- No transmission of user-content data off-device without explicit opt-in.
- No use of user-content data, directly or indirectly, in the training of AI/ML models. (This also honours the Reticulum License clause 2.)

## No hidden backdoors

The founding vendor commits that:

- No secret cryptographic key allows the vendor, a government, or any third party to decrypt user-to-user messages.
- The firmware architecture prevents such a key from being effective even if silently added; end-to-end encryption is enforced at the user endpoints.
- Reproducible builds and public SBOMs make silent backdoor introduction detectable.

## Bankruptcy / vendor-loss plan

If the founding vendor entity dissolves without a successor, the escrowed release-signing key `RelKey_C` (see `05_SECURITY_MODEL.md` §13.4) and the trademark portfolio will pass, per pre-agreed escrow instrument, to the SS-SP Foundation or, if none exists, to an independent open-source steward organisation. Devices in the field will retain full functionality on the last shipped firmware indefinitely.

## Amendments

This document may only be amended:

- With Steering Committee supermajority (4/5) approval, and
- 12 months of public notice, and
- Only in ways that **strengthen** community protections. Weakening amendments are prohibited.

## Signatures

_(Will be countersigned at first tagged release; signatures anchored in Sigstore transparency log.)_

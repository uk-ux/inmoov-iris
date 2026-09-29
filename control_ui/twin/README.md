# InMoov head digital-twin workbench

Open http://127.0.0.1:8765/twin/ using the existing local control UI server.

33 official STL files downloaded from the i2Head / i2Eyes galleries, excluding
skin casting molds. Three.js 0.180.0 is bundled locally (MIT; see vendor/package/LICENSE).
InMoov geometry © Gaël Langevin, CC BY-NC 3.0. Source URLs, original bounds,
triangle counts and SHA-256 checksums are in assets/manifest.json.

This is a detailed mesh inspection and assembly workbench, not a fully verified
mechanical twin. Models are unscaled and centered for placement; assembly
translations, rotations and preview motion axes are estimates. All 33 STLs are now
split into 76 connected components by hardware/prepare_assembly_parts.py.
Every triangle is retained exactly once; originals remain untouched. Derived
checksums, original parent checksums and bounds are in assets/components.json.
The eye plates each contain 16 pieces, now separately positioned, with approximate
eyeball gaze and eyelid pivot motion. No silicone skin or
servo/screw hardware models are fabricated. Jaw, cheeks, eyebrows and forehead
have approximate motion previews. Neck rotation has no assigned free channel.

Part selection, isolation, skull visibility, wireframe, labels, exploded layout,
parts layout, zoom/orbit and editable assembly transforms are available. Saved
layout overrides live in localStorage on this origin and can be exported as JSON.
Component poses use inmoov-twin-layout-v2-components; old v1 overrides are retained
but not loaded because they refer to whole print plates, not individual pieces.
Each moving mechanism links to the existing calibration panel on the control page.

The new page sends no serial commands. BroadcastChannel receives successful
control-page serial writes when live following is enabled. This is commanded
state, not actual shaft feedback. Onboard autonomous animations do not stream
positions, so their state cannot be reconstructed as measured motion.

Next accuracy work: verify source revision against actual prints,
measure/align mounting surfaces and joint pivots, add linkage constraints,
and validate each neutral/endpoint against photos or measurements of the robot.

Assembly correction status (2026-09-16): duplicate paired print plates removed,
jaw/eye orientations corrected, brow/forehead pieces separated, initial symmetric
skull poses rebuilt. Shell seams and smaller linkages still have visible placement
errors: do not present this as a perfectly assembled or engineering-verified twin.
Official reference images are in hardware/assembly-references. An assembled CAD
file would give authoritative transforms; front/side/rear photos and dimensions
can support a manual alignment to this particular physical head.

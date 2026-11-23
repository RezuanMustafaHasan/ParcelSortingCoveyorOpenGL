# Academic LaTeX report

Main source: `conveyor_sorting_report.tex`

This revision retains Libre Baskerville, the original technical content and all 25 screenshot placements. It uses formal section headings, black heading text, plain explanatory paragraphs and 1.5-line body spacing. The shading section includes the classic Phong reflection equation, Phong normal interpolation and Gouraud color interpolation, with the project's Blinn-Phong implementation distinguished explicitly. Proposed and implemented features remain clearly distinguished.

## Compile

Use **XeLaTeX**, not pdfLaTeX. Keep `sections/`, `assets/`, and `fonts/` next to the main `.tex` file. From this folder:

```bash
xelatex -interaction=nonstopmode -halt-on-error conveyor_sorting_report.tex
xelatex -interaction=nonstopmode -halt-on-error conveyor_sorting_report.tex
```

Run twice to resolve internal page references. No BibTeX, shell escape or access to the application project is needed. On Overleaf, upload the complete bundle, choose `conveyor_sorting_report.tex` as the main file, and set the compiler to **XeLaTeX**.

Libre Baskerville Regular, Bold, Italic and Bold Italic are included under `fonts/`, together with the SIL Open Font License. Body text, headings, tables and captions use this font. Latin Modern is used for mathematical symbols and code. Font files are loaded locally; no system installation is required.

Common TeX packages: fontspec, lmodern, geometry, microtype, amsmath, amssymb, graphicx, booktabs, tabularx, array, xcolor, enumitem, fancyhdr, titlesec, tikz, caption, float, listings, setspace, hyperref.

## Contents and editing

- Institution, department, student name, roll, year, semester, course and date are on the title page in the main `.tex` file. The KUET logo is included as `assets/kuet_logo.png`.
- The ten retained sections are in `sections/01_visual_tour.tex` through `sections/10_controls.tex`.
- The results/tests/limitations, implementation reproduction, and sources/references/template sections have been removed, together with the final access-date paragraph. The overhead screenshot is retained in the camera subsection.
- Screenshots, exact crop rectangles and capture state/camera metadata are in `assets/`.
- Image aspect ratios are preserved. Callout arrows are vector overlays in the LaTeX source; the screenshot pixels were not altered apart from lossless crops.
- `tools/` contains the temporary capture helper sources, a reproduction script and instructions. These are only needed to regenerate screenshots from the original application, not to compile the report.
- `implementation_snapshot.txt` identifies the unchanged project sources; `validation.txt` records checks and capture evidence.
- `assets/original_proposal.jpeg` preserves the supplied proposal. The complete set of full-resolution frames is included even when the report uses a crop.

Preserve these implementation distinctions: destinations are assigned at spawning; the scanner is simulated; the gate uses coordinated paths instead of contact forces; default Phong shading uses Blinn-Phong specular. Some objects, including low decorative cargo in the bins, are static. Collected deliveries free their slots rather than accumulating forever.

From the repository root, first run `cd docs/report`, then use the compile commands above. Generated PDFs are ignored by Git; compile locally to obtain the report. Earlier exports and report archives remain in the workspace's ignored `output/` folder when available.

The implementation snapshot and validation notes describe the sources inspected for the October 2026 report. They are historical capture evidence; the repository README and build configuration may have changed since then.

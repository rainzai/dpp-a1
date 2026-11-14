#!/usr/bin/env python3
"""Makes all report figures from the DAS-5 measurements.

Usage: python3 plot.py [results_dir]
Reads wave.csv and sieve.csv from results_dir (default: ./results) and writes
speedup.pdf, efficiency.pdf, schedules.pdf and sieve.pdf next to them.
Every point is the median of the repeated runs; the error bars show the
fastest and slowest run.
"""

import csv
import os
import statistics
import sys
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

THREADS = [1, 2, 4, 6, 8, 12, 14, 16]
SIZES = [10**3, 10**4, 10**5, 10**6, 10**7]
SIZE_LABEL = {10**3: "$10^3$", 10**4: "$10^4$", 10**5: "$10^5$",
              10**6: "$10^6$", 10**7: "$10^7$"}

# Categorical colours, always in this order (colour-blind safe).
COLORS = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"]
HATCHES = ["", "//", "\\\\", "..", "xx"]
INK = "#52514e"   # axis and label colour
GRID = "#e4e3df"  # recessive grid lines

plt.rcParams.update({
    "font.size": 9,
    "axes.edgecolor": INK,
    "axes.labelcolor": INK,
    "xtick.color": INK,
    "ytick.color": INK,
    "axes.grid": True,
    "grid.color": GRID,
    "grid.linewidth": 0.6,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "legend.frameon": False,
    "lines.linewidth": 1.6,
    "lines.markersize": 4,
})


def read_csv(path):
    """Returns the rows of a CSV file, skipping runs without a time."""
    with open(path, newline="") as f:
        return [r for r in csv.DictReader(f) if r["seconds"]]


def stats(times):
    """Returns (median, distance to fastest run, distance to slowest run)."""
    med = statistics.median(times)
    return med, med - min(times), max(times) - med


def wave_times(rows):
    """Groups wave runs by (program, i_max, threads, schedule, chunk)."""
    groups = defaultdict(list)
    for r in rows:
        key = (r["program"], int(r["i_max"]), int(r["threads"]),
               r["schedule"], r["chunk"])
        groups[key].append(float(r["seconds"]))
    return groups


def parallel_key(program, size, threads):
    """Key of the main experiment: pthreads, or OpenMP with schedule static."""
    sched = "static" if program == "openmp" else "-"
    return (program, size, threads, sched, "-")


def plot_scaling(groups, out_path, efficiency):
    """One panel per problem size with pthreads and OpenMP side by side."""
    seq = {n: statistics.median(groups[("seq", n, 1, "-", "-")])
           for n in SIZES}
    fig, axes = plt.subplots(1, len(SIZES), figsize=(10, 2.6), sharey=True)
    programs = [("pthreads", "pthreads"), ("openmp", "OpenMP (static)")]

    for ax, n in zip(axes, SIZES):
        ideal = [1.0] * len(THREADS) if efficiency else THREADS
        ax.plot(THREADS, ideal, color=INK, linestyle="--", linewidth=1,
                label="ideal")
        for i, (prog, label) in enumerate(programs):
            ys, lo, hi = [], [], []
            for p in THREADS:
                times = groups[parallel_key(prog, n, p)]
                # speedup = T_seq / T_par, so the fastest run gives the
                # highest speedup.
                values = [seq[n] / t / (p if efficiency else 1)
                          for t in times]
                med, down, up = stats(values)
                ys.append(med)
                lo.append(down)
                hi.append(up)
            ax.errorbar(THREADS, ys, yerr=[lo, hi], color=COLORS[i],
                        marker="o", capsize=2, elinewidth=0.8, label=label)
        ax.set_title(f"$i_{{max}}$ = {SIZE_LABEL[n]}", color=INK)
        ax.set_xlabel("threads")
        ax.set_xticks([1, 4, 8, 12, 16])

    axes[0].set_ylabel("efficiency" if efficiency else "speedup")
    if not efficiency:
        axes[0].set_ylim(0, 17)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=3,
               bbox_to_anchor=(0.5, 1.08))
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")
    plt.close(fig)


def plot_schedules(groups, out_path):
    """Grouped bars: OpenMP time per schedule at 10^6 points."""
    schedules = [("static", "-", "static"),
                 ("static", "1000", "static,1000"),
                 ("dynamic", "1000", "dynamic,1000"),
                 ("dynamic", "10000", "dynamic,10000"),
                 ("guided", "-", "guided")]
    threads = [4, 8, 16]
    width = 0.16
    fig, ax = plt.subplots(figsize=(5.5, 3))
    for i, (sched, chunk, label) in enumerate(schedules):
        xs, ys, lo, hi = [], [], [], []
        for j, p in enumerate(threads):
            med, down, up = stats(groups[("openmp", 10**6, p, sched, chunk)])
            xs.append(j + (i - 2) * (width + 0.01))
            ys.append(med)
            lo.append(down)
            hi.append(up)
        ax.bar(xs, ys, width, yerr=[lo, hi], color=COLORS[i],
               hatch=HATCHES[i], edgecolor="white", linewidth=0.5,
               error_kw={"elinewidth": 0.8, "capsize": 2, "ecolor": INK},
               label=label)
    ax.set_xticks(range(len(threads)))
    ax.set_xticklabels([f"{p} threads" for p in threads])
    ax.set_ylabel("time (s), lower is better")
    ax.grid(axis="x", visible=False)
    ax.legend(ncol=3, loc="upper center", bbox_to_anchor=(0.5, 1.25))
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")
    plt.close(fig)


def plot_sieve(rows, out_path):
    """Sieve run time against the number of cores, log scale."""
    groups = defaultdict(list)
    for r in rows:
        groups[(int(r["primes"]), int(r["cores"]))].append(float(r["seconds"]))
    cores = sorted({c for _, c in groups})
    fig, ax = plt.subplots(figsize=(4.5, 3))
    for i, n in enumerate(sorted({n for n, _ in groups})):
        ys, lo, hi = [], [], []
        for c in cores:
            med, down, up = stats(groups[(n, c)])
            ys.append(med)
            lo.append(down)
            hi.append(up)
        ax.errorbar(cores, ys, yerr=[lo, hi], color=COLORS[i], marker="o",
                    capsize=2, elinewidth=0.8, label=f"{n} primes")
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xticks(cores)
    ax.set_xticklabels([str(c) for c in cores])
    ax.set_xlabel("cores (taskset)")
    ax.set_ylabel("time (s)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")
    plt.close(fig)


def main():
    results = sys.argv[1] if len(sys.argv) > 1 else "results"
    wave = wave_times(read_csv(os.path.join(results, "wave.csv")))
    plot_scaling(wave, os.path.join(results, "speedup.pdf"), efficiency=False)
    plot_scaling(wave, os.path.join(results, "efficiency.pdf"),
                 efficiency=True)
    plot_schedules(wave, os.path.join(results, "schedules.pdf"))
    plot_sieve(read_csv(os.path.join(results, "sieve.csv")),
               os.path.join(results, "sieve.pdf"))
    print("Wrote speedup.pdf, efficiency.pdf, schedules.pdf and sieve.pdf"
          f" to {results}/")


if __name__ == "__main__":
    main()

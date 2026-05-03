import os
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATASETS = ["dataset-10k", "dataset-100k"]
OUTPUT_DIR = os.path.join(BASE_DIR, "graphs")

os.makedirs(OUTPUT_DIR, exist_ok=True)


# -----------------------------
# Normalize command → DB name
# -----------------------------
def normalize_command(cmd: str) -> str:
    cmd = cmd.lower()
    if "psql" in cmd or "postgres" in cmd:
        return "PostgreSQL"
    elif "sqlite" in cmd:
        return "SQLite"
    elif "mainapp" in cmd or "minisql" in cmd:
        return "MiniSQL"
    else:
        return "Unknown"


def load_csv(path):
    df = pd.read_csv(path)
    df["db"] = df["command"].apply(normalize_command)
    return df

def pretty_name(op):
    return op.replace("_", " ").title()

# -----------------------------
# Bar chart with error bars
# -----------------------------
def generate_bar_chart(title, df, output_path):
    df = df.sort_values("mean")

    labels = df["db"].tolist()
    means = df["mean"].tolist()
    stddevs = df["stddev"].tolist()

    x = range(len(labels))

    plt.figure(figsize=(6, 4))
    plt.bar(x, means, yerr=stddevs, capsize=5)

    plt.xticks(x, labels)  # <-- force clean labels

    plt.ylabel("Time (seconds)")
    plt.title(pretty_name(title))

    plt.savefig(output_path, format="svg", bbox_inches="tight")
    plt.close()
# -----------------------------
# Scaling chart (10k vs 100k)
# -----------------------------
def generate_scaling_chart(title, df_10k, df_100k, output_path):
    plt.figure(figsize=(6, 4))

    dbs = ["PostgreSQL", "SQLite", "MiniSQL"]

    for db in dbs:
        row10 = df_10k[df_10k["db"] == db]
        row100 = df_100k[df_100k["db"] == db]

        if row10.empty or row100.empty:
            continue  # avoid crash

        t10 = row10["mean"].values[0]
        t100 = row100["mean"].values[0]

        plt.plot([10_000, 100_000], [t10, t100], marker='o', label=db)

    plt.xscale("log")
    plt.yscale("log")

    plt.xlabel("Dataset size (rows)")
    plt.ylabel("Time (seconds)")
    plt.title(title)
    plt.legend()

    plt.savefig(output_path, format="svg", bbox_inches="tight")
    plt.close()


# -----------------------------
# Speedup chart (relative to fastest)
# -----------------------------
def generate_speedup_chart(title, df, output_path):
    fastest = df["mean"].min()
    df["speedup"] = df["mean"] / fastest

    labels = df["db"]
    speedups = df["speedup"]

    plt.figure(figsize=(6, 4))
    plt.bar(labels, speedups)

    plt.ylabel("time / fastest run ")
    plt.title(title)

    plt.savefig(output_path, format="svg", bbox_inches="tight")
    plt.close()


# -----------------------------
# Main
# -----------------------------
import os

def main():
    datasets = ["dataset-10k", "dataset-100k"]
    
    # 1. Process each directory independently for Bar Charts
    for dataset in datasets:
        dataset_path = os.path.join(BASE_DIR, dataset)
        
        # Guard clause in case directory doesn't exist
        if not os.path.exists(dataset_path):
            continue

        for file in os.listdir(dataset_path):
            if file.endswith(".csv"):
                op = file.replace(".csv", "")
                file_path = os.path.join(dataset_path, file)
                
                # Load and generate individual bar chart
                df = load_csv(file_path)
                suffix = "10k" if "10k" in dataset else "100k"
                
                generate_bar_chart(
                    f"{op} ({suffix} rows)",
                    df,
                    os.path.join(OUTPUT_DIR, f"{op}_{suffix}.svg")
                )

    # 2. Process Scaling Charts (Only where pairs exist)
    # We can use the 10k folder as the reference
    path_10k_dir = os.path.join(BASE_DIR, "dataset-10k")
    if os.path.exists(path_10k_dir):
        for file in os.listdir(path_10k_dir):
            if file.endswith(".csv"):
                op = file.replace(".csv", "")
                
                path_10k = os.path.join(BASE_DIR, "dataset-10k", file)
                path_100k = os.path.join(BASE_DIR, "dataset-100k", file)

                if os.path.exists(path_100k):
                    df_10k = load_csv(path_10k)
                    df_100k = load_csv(path_100k)
                    
                    generate_scaling_chart(
                        f"{op} scaling",
                        df_10k,
                        df_100k,
                        os.path.join(OUTPUT_DIR, f"{op}_scaling.svg")
                    )
                else:
                    print(f"Skipping scaling chart for {op}: 100k file missing.")


if __name__ == "__main__":
    main()
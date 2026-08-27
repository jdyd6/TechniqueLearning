from pathlib import Path

import numpy as np
import tensorflow as tf


SEED = 42
SAMPLE_COUNT = 1200
WINDOW_SIZE = 32
CHANNEL_COUNT = 3

np.random.seed(SEED)
tf.random.set_seed(SEED)


def create_rest_sample() -> np.ndarray:
    """生成接近静止状态的三轴窗口。"""
    base = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    noise = np.random.normal(
        loc=0.0,
        scale=0.03,
        size=(WINDOW_SIZE, CHANNEL_COUNT),
    )
    return (base + noise).astype(np.float32)


def create_motion_sample() -> np.ndarray:
    """生成带有周期变化的三轴窗口。"""
    time_axis = np.linspace(0.0, 2.0 * np.pi, WINDOW_SIZE, dtype=np.float32)
    phase = np.random.uniform(0.0, 2.0 * np.pi)
    signal = np.stack(
        [
            0.7 * np.sin(time_axis + phase),
            0.4 * np.sin(2.0 * time_axis + phase),
            1.0 + 0.3 * np.cos(time_axis + phase),
        ],
        axis=1,
    )
    noise = np.random.normal(
        loc=0.0,
        scale=0.05,
        size=(WINDOW_SIZE, CHANNEL_COUNT),
    )
    return (signal + noise).astype(np.float32)


def create_dataset() -> tuple[np.ndarray, np.ndarray]:
    """生成平衡的二分类数据集。"""
    samples = []
    labels = []

    for index in range(SAMPLE_COUNT):
        is_motion = index % 2 == 1
        sample = create_motion_sample() if is_motion else create_rest_sample()
        samples.append(sample)
        labels.append(int(is_motion))

    features = np.asarray(samples, dtype=np.float32)
    targets = np.asarray(labels, dtype=np.int32)

    order = np.random.permutation(len(features))
    return features[order], targets[order]


features, targets = create_dataset()

train_end = int(len(features) * 0.7)
validation_end = int(len(features) * 0.85)

x_train = features[:train_end]
y_train = targets[:train_end]
x_validation = features[train_end:validation_end]
y_validation = targets[train_end:validation_end]
x_test = features[validation_end:]
y_test = targets[validation_end:]

model = tf.keras.Sequential(
    [
        tf.keras.layers.Input(shape=(WINDOW_SIZE, CHANNEL_COUNT)),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(16, activation="relu"),
        tf.keras.layers.Dense(2, activation="softmax"),
    ]
)

model.compile(
    optimizer="adam",
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"],
)

model.summary()

model.fit(
    x_train,
    y_train,
    validation_data=(x_validation, y_validation),
    epochs=12,
    batch_size=32,
    verbose=2,
)

test_loss, test_accuracy = model.evaluate(x_test, y_test, verbose=0)
print(f"test loss: {test_loss:.4f}")
print(f"test accuracy: {test_accuracy:.4f}")

prediction = model.predict(x_test[:1], verbose=0)[0]
print("prediction probabilities:", prediction)
print("predicted class:", int(np.argmax(prediction)))
print("expected class:", int(y_test[0]))

artifact_dir = Path("artifacts")
artifact_dir.mkdir(exist_ok=True)
model.save(artifact_dir / "first_model.keras")
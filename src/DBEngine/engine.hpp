#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include "../interpreter/execution_context.hpp"

// Static class
struct DBEngine
{

  // Delete the default constructor to prevent instantiation
  DBEngine() = delete;

  // Delete copy constructor and assignment operator to prevent copying
  DBEngine(const DBEngine &) = delete;
  DBEngine &operator=(const DBEngine &) = delete;

  // Static class
  struct FileHandler
  {
    static const std::string SCHEMA_BASE_DIRECTORY;
    static const std::string METADATA_BASE_DIRECTORY;
    static const std::string DATASTORAGE_BASE_DIRECTORY;

    // Delete the default constructor to prevent instantiation
    FileHandler() = delete;

    // Delete copy constructor and assignment operator to prevent copying
    FileHandler(const FileHandler &) = delete;
    FileHandler &operator=(const FileHandler &) = delete;

    static void createUntypedTable(CreateUntypedTableNode &node, ExecutionContext &ctx)
    {
      if (ctx.untyped_tables.find(node.tableName) != ctx.untyped_tables.end())
      {
        std::cerr << "Error: Table " << node.tableName << " already exists.\n";
        return;
      }
      // Store metadata on disk
      // Base directory for schema
      std::filesystem::create_directories(FileHandler::METADATA_BASE_DIRECTORY); // ensures parents exist

      // Path for this table
      std::string table_path = FileHandler::METADATA_BASE_DIRECTORY + "/" + node.tableName;
      std::filesystem::create_directories(table_path);

      // Write metadata file
      std::string metadata_path = table_path + "/metadata.txt";
      std::ofstream file(metadata_path);
      if (!file.is_open())
      {
        std::cerr << "Error: Could not create metadata file for table "
                  << node.tableName << "\n";
        return;
      }

      for (auto c : node.columns)
      {
        auto col = dynamic_cast<UntypedColumnDefNode *>(c);
        if (col)
        {
          file << col->name;
          if (!col->modifiers.empty())
            file << " ";
          for (size_t i = 0; i < col->modifiers.size(); ++i)
          {
            file << col->modifiers[i];
            if (i + 1 < col->modifiers.size())
              file << ",";
          }
          file << "\n";
        }
      }
      file.close();

      // Store schema in context
      std::vector<UntypedColumnDefNode *> cols;
      cols.reserve(node.columns.size());
      for (auto *col : node.columns)
      {
        auto casted_col = dynamic_cast<UntypedColumnDefNode *>(col);
        if (casted_col)
        {
          cols.push_back(new UntypedColumnDefNode(*casted_col)); // deep copy
        }
      }
      std::cout << "Created table " << node.tableName
                << " with " << cols.size() << " columns\n";

      ctx.untyped_tables[node.tableName] = std::move(cols);
    }

    static void dropTable(DropTableNode &node, ExecutionContext &ctx)
    {
      std::string table_metadata_directory = FileHandler::METADATA_BASE_DIRECTORY + "/" + node.tableName;

      if (!std::filesystem::exists(table_metadata_directory))
      {
        std::cerr << "Error: metadata for table \"" << node.tableName << "\" does not exist on disk.\n";
        return;
      }

      try
      {
        // Remove directory and all metadata
        std::filesystem::remove_all(table_metadata_directory);
        // TODO Remove datastorage files

        // Now remove from execution context if it exists
        auto it = ctx.untyped_tables.find(node.tableName);
        if (it != ctx.untyped_tables.end())
        {
          // free the schema memory before erasing
          for (auto col : it->second)
          {
            delete col;
          }
          ctx.untyped_tables.erase(it);
        }

        std::cout << "Table \"" << node.tableName << "\" dropped successfully.\n";
      }
      catch (const std::filesystem::filesystem_error &e)
      {
        std::cerr << "Filesystem error while dropping table: " << e.what() << "\n";
      }
    }

    static void insertData(InsertNode &node, ExecutionContext &ctx)
    {
      std::filesystem::create_directories(FileHandler::DATASTORAGE_BASE_DIRECTORY); // ensures parent exists

      // Path for this table
      const std::string table_datastorage_dir = FileHandler::DATASTORAGE_BASE_DIRECTORY + node.tableName;
      std::filesystem::create_directories(table_datastorage_dir);

      // TODO File logic
    }
  };
};

// Static field initialization

const std::string DBEngine::FileHandler::SCHEMA_BASE_DIRECTORY = "../src/schema";
const std::string DBEngine::FileHandler::METADATA_BASE_DIRECTORY = DBEngine::FileHandler::SCHEMA_BASE_DIRECTORY + "/metadata";
const std::string DBEngine::FileHandler::DATASTORAGE_BASE_DIRECTORY = DBEngine::FileHandler::SCHEMA_BASE_DIRECTORY + "/data";

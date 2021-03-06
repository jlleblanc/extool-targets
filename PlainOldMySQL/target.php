<?php
/**
 * Extool Plain Old MySQL Target
 * 
 * Copyright 2010 Joseph LeBlanc
 * See LICENSE file for licensing details.
 * 
 */
namespace Extool\Target;

class PlainOldMySQL implements TargetInterface
{
	private $rep;

	function getConfiguration()
	{
		$fields = new \Extool\Representation\Fields(array());

		$config = new Configuration($fields);

		return $config;
	}

	public function setConfiguration(Configuration $configuration)
	{
		
	}

	public function setRepresentation(\Extool\Representation\Representation $representation)
	{
		$this->rep = $representation;
	}

	public function generate()
	{
		$files = new \Extool\Helpers\FilePackage();

		$install_file = new \Extool\Helpers\File();
		$uninstall_file = new \Extool\Helpers\File();

		foreach ($this->rep->tables as $table) {
			$table->createDefaultKey();

			$mysql = new \Extool\Helpers\MySQL($table);
			$install_file->appendContents($mysql->generateCreate() . "\n\n");
			$uninstall_file->appendContents($mysql->generateDrop() . "\n");
		}

		$files->addFile('admin/install.mysql.sql', $install_file);
		$files->addFile('admin/uninstall.mysql.sql', $uninstall_file);

		return $files;
	}
}
